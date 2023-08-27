#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "Kamstrup303WA02.h"
#include "Pwm.h"

namespace esphome {
namespace heatmeter_mbus {

class MbusSensor : public sensor::Sensor {
  public:
    MbusSensor(uint8_t index) : index_(index) {}

  private:
    uint8_t index_;
};
class HeatMeterMbus : public Component, public uart::UARTDevice {
  public:  
    HeatMeterMbus() : kamstrup(this) {}
    
    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override;

    sensor::Sensor* create_sensor(uint8_t index) {
      return new MbusSensor(index);
    }
    void enableMbus();
    void disableMbus();
    void readMbus();

  private:
    Pwm pwm;
    Kamstrup303WA02 kamstrup;
    bool updateRequested { false };
    bool mbusEnabled { true };

    static void read_mbus_task_loop(void* params);
    static esp_err_t initializeAndEnablePwm(Pwm* pwm);
};

} //namespace heatmeter_mbus
} //namespace esphome