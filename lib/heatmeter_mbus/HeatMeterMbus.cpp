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
              for (auto it = mbus_meter_data.data_blocks->begin(); it != mbus_meter_data.data_blocks->end(); ++it) {
                auto data_block = *it;
                ESP_LOGI(TAG, "-- Index:\t\t%d --", data_block->index);
                ESP_LOGI(TAG, "Function:\t\t%d", data_block->function);
                ESP_LOGI(TAG, "Storage number:\t%d", data_block->storage_number);
                ESP_LOGI(TAG, "Unit:\t\t\t%d", data_block->unit);
                ESP_LOGI(TAG, "Ten power:\t\t%d", data_block->ten_power);
                ESP_LOGI(TAG, "Data length:\t\t%d", data_block->data_length);
                ESP_LOGI(TAG, "----------------------------");
              }
              heatMeterMbus->have_dumped_data_blocks_ = true;
            }
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
      LOG_SENSOR("  ", "T1 Actual", this->t1_actual_sensor_);
      LOG_SENSOR("  ", "Heat Energy E1", this->heat_energy_e1_sensor_);
      LOG_SENSOR("  ", "Volume V1", this->volume_v1_sensor_);
      LOG_SENSOR("  ", "Energy E8 Inlet", this->energy_e8_inlet_sensor_);
      LOG_SENSOR("  ", "Energy E9 Outlet", this->energy_e9_outlet_sensor_);
      LOG_SENSOR("  ", "Operating Hours", this->operating_hours_sensor_);
      LOG_SENSOR("  ", "Error Hour Counter", this->error_hour_counter_sensor_);
      LOG_SENSOR("  ", "T1 Actual", this->t1_actual_sensor_);
      LOG_SENSOR("  ", "T2 Actual", this->t2_actual_sensor_);
      LOG_SENSOR("  ", "T1 - T2", this->t1_minus_t2_sensor_);
      LOG_SENSOR("  ", "Power E1 / E3", this->power_e1_over_e3_sensor_);
      LOG_SENSOR("  ", "Power Max Month", this->power_max_month_sensor_);
      LOG_SENSOR("  ", "Flow V1 Actual", this->flow_v1_actual_sensor_);
      LOG_SENSOR("  ", "Flow V1 Max Month", this->flow_v1_max_month_sensor_);
      LOG_SENSOR("  ", "Heat Energy E1 Old", this->heat_energy_e1_old_sensor_);
      LOG_SENSOR("  ", "Volume V1 Old", this->volume_v1_old_sensor_);
      LOG_SENSOR("  ", "Energy E8 Inlet Old", this->energy_e8_inlet_old_sensor_);
      LOG_SENSOR("  ", "Energy E9 Outlet Old", this->energy_e9_outlet_old_sensor_);
      LOG_SENSOR("  ", "Power Max Year Old", this->power_max_year_old_sensor_);
      LOG_SENSOR("  ", "Flow V1 Max Year Old", this->flow_v1_max_year_old_sensor_);
      LOG_SENSOR("  ", "Log Year", this->log_year_sensor_);
      LOG_SENSOR("  ", "Log Month", this->log_month_sensor_);
      LOG_SENSOR("  ", "Log Day", this->log_day_sensor_);

      LOG_BINARY_SENSOR("  ", "No Voltage Supply", this->info_no_voltage_supply_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T1 Above Measuring Range or Disconnected", this->info_t1_above_range_or_disconnected_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T2 Above Measuring Range or Disconnected", this->info_t2_above_range_or_disconnected_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T1 Below Measuring Range or Short-circuited", this->info_t1_below_range_or_shorted_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "T2 Below Measuring Range or Short-circuited", this->info_t2_below_range_or_shorted_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "Invalid Temperature Difference (T1 - T2)", this->info_invalid_temp_difference_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "V1 Air", this->info_v1_air_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "V1 Wrong Flow Direction", this->info_v1_wrong_flow_direction_binary_sensor_);
      LOG_BINARY_SENSOR("  ", "V1 > Qs For More Than An Hour", this->info_v1_greater_than_qs_more_than_hour_binary_sensor_);
    }
  } // namespace warmtemetermbus
} // namespace esphome

#endif // UNIT_TEST
