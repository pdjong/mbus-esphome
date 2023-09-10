#ifndef UNIT_TEST

#include "esphome/core/log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include "HeatMeterMbus.h"
#include "Kamstrup303WA02.h"
#include "EspArduinoUartInterface.h"

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {
    static const char *TAG = "heatmetermbus.sensor";

    bool pwmInitialized { false };
    bool pwmEnabled { false };

    HeatMeterMbus::HeatMeterMbus() {
      EspArduinoUartInterface *uart_interface = new EspArduinoUartInterface(this);
      this->kamstrup = new Kamstrup303WA02(uart_interface);
    }

    void HeatMeterMbus::setup()
    {
      ESP_LOGI(TAG, "setup()");
      if (ESP_OK != initializeAndEnablePwm(&pwm))
      {
        ESP_LOGE(TAG, "Error initializing and enabling PWM");
      }

      xTaskCreatePinnedToCore(HeatMeterMbus::read_mbus_task_loop,
                        "mbus_task", // name
                        10000,       // stack size (in words)
                        this,        // input params
                        1,           // priority
                        nullptr,     // Handle, not needed
                        0            // core
      );
    }

    esp_err_t HeatMeterMbus::initializeAndEnablePwm(Pwm* pwm)
    {
      esp_err_t configResult = pwm->initialize(32, 18000, 0.85f);
      if (ESP_OK != configResult)
      {
        ESP_LOGE(TAG, "Error initializing PWM: %d", configResult);
        return configResult;
      }
      else
      {
        ESP_LOGI(TAG, "Initialized PWM");
        pwmInitialized = true;
      }

      esp_err_t pwmEnableResult = pwm->enable();
      if (ESP_OK != pwmEnableResult)
      {
        ESP_LOGE(TAG, "Error enabling PWM channel");
      }
      else
      {
        ESP_LOGI(TAG, "Enabled PWM channel");
        pwmEnabled = true;
      }
      return pwmEnableResult;
    }

    void HeatMeterMbus::read_mbus_task_loop(void* params)
    {
      HeatMeterMbus *heatMeterMbus = reinterpret_cast<HeatMeterMbus*>(params);

      while (true)
      {
        const bool shouldReadNow = heatMeterMbus->updateRequested && heatMeterMbus->mbusEnabled;
        if (heatMeterMbus->updateRequested && !heatMeterMbus->mbusEnabled)
        {
          ESP_LOGD(TAG, "Read Mbus requested but Mbus disabled");
        }
        if (shouldReadNow)
        {
          // Let's request data, and wait for its results :-)
          Kamstrup303WA02::MbusMeterData mbus_meter_data;
          bool read_is_successful { heatMeterMbus->kamstrup->read_meter_data(&mbus_meter_data, heatMeterMbus->address) };

          if (read_is_successful) {
            ESP_LOGI(TAG, "Successfully read meter data");

            if (!heatMeterMbus->have_dumped_data_blocks_) {
              for (auto data_block : *mbus_meter_data.data_blocks) {
                ESP_LOGI(TAG, "-- Index:\t\t\t%d --", data_block->index);
                ESP_LOGI(TAG, "Function:\t\t\t%d", data_block->function);
                ESP_LOGI(TAG, "Storage number:\t\t%d", data_block->storage_number);
                ESP_LOGI(TAG, "Unit:\t\t\t%d", data_block->unit);
                ESP_LOGI(TAG, "Ten power:\t\t\t%d", data_block->ten_power);
                ESP_LOGI(TAG, "Data length:\t\t%d", data_block->data_length);
                ESP_LOGI(TAG, "-------------------------------");
              }
              heatMeterMbus->have_dumped_data_blocks_ = true;
            }

            for (auto it = heatMeterMbus->sensors_.begin(); it != heatMeterMbus->sensors_.end(); ++it) {
              MbusSensor *sensor = *it;
              for (auto data_block_it = mbus_meter_data.data_blocks->begin(); data_block_it != mbus_meter_data.data_blocks->end(); ++data_block_it) {
                Kamstrup303WA02::DataBlock *data_block = *data_block_it;
                if (data_block->index == sensor->index_) {
                  ESP_LOGI(TAG, "Found matching data block");
                  Kamstrup303WA02::DataBlock *matching_data_block { data_block };
                  switch (matching_data_block->data_length) {
                    case 2: {
                      int16_t *raw_value = reinterpret_cast<int16_t*>(matching_data_block->binary_data);
                      float value = static_cast<float>(*raw_value * pow(10, matching_data_block->ten_power));
                      sensor->publish_state(value);
                      break;
                    }
                    case 4: {
                      int32_t *raw_value = reinterpret_cast<int32_t*>(matching_data_block->binary_data);
                      float value = static_cast<float>(*raw_value * pow(10, matching_data_block->ten_power));
                      sensor->publish_state(value);
                      break;
                    }
                    default:
                      break;
                  }
                  break;
                }
              }
            }

            // Deallocate data_blocks
            for (auto it = mbus_meter_data.data_blocks->begin(); it != mbus_meter_data.data_blocks->end(); ++it) {
              Kamstrup303WA02::DataBlock *data_block = *it;
              if (data_block->binary_data != nullptr) {
                delete[] data_block->binary_data;
                data_block->binary_data = nullptr;
              }
              delete data_block;
            }
            delete mbus_meter_data.data_blocks;
            mbus_meter_data.data_blocks = nullptr;
          }
          else {
            ESP_LOGE(TAG, "Did not successfully read meter data");
          }
          heatMeterMbus->updateRequested = false;
        }
        else
        {
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
      }
    }

    void HeatMeterMbus::enableMbus() {
      ESP_LOGI(TAG, "Enabling Mbus");
      pwm.enable();
      mbusEnabled = true;
    }

    void HeatMeterMbus::disableMbus() {
      ESP_LOGI(TAG, "Disabling Mbus");
      pwm.disable();
      mbusEnabled = false;
      this->have_dumped_data_blocks_ = false;
    }

    void HeatMeterMbus::readMbus()
    {
      updateRequested = true;
    }

    float HeatMeterMbus::get_setup_priority() const
    {
      // After UART bus
      return setup_priority::BUS - 1.0f;
    }

    void HeatMeterMbus::dump_config()
    {
      ESP_LOGCONFIG(TAG, "HeatMeterMbus sensor");
    }
  } // namespace warmtemetermbus
} // namespace esphome

#endif // UNIT_TEST
