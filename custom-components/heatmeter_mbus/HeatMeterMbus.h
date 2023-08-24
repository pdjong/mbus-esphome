#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
// #include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/uart/uart.h"
// #include "Kamstrup303WA02.h"
// #include "Pwm.h"

namespace esphome {
namespace heatmeter_mbus {

class MbusSensor : public sensor::Sensor {
  public:
    MbusSensor(uint8_t index) : index_(index) {}

  private:
    uint8_t index_;
};

#define HEATMETERMBUS_METER_SENSOR(name) \
 protected: \
  sensor::Sensor *name##_sensor_{nullptr}; \
\
 public: \
  void set_##name##_sensor(sensor::Sensor *(name)) { this->name##_sensor_ = name; }

#define HEATMETERMBUS_METER_BINARYSENSOR(name) \
 protected: \
  binary_sensor::BinarySensor *name##_binary_sensor_{nullptr}; \
\
 public: \
  void set_##name##_binary_sensor(binary_sensor::BinarySensor *(name)) { this->name##_binary_sensor_ = name; }

class HeatMeterMbus : public Component, public uart::UARTDevice {
  public:  
    HeatMeterMbus() /*: kamstrup(this) */ {}
    
    // HEATMETERMBUS_METER_SENSOR(t1_actual)
    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override;

    sensor::Sensor* create_sensor(uint8_t index) {
      return new MbusSensor(index);
    }
    // void enableMbus();
    // void disableMbus();
    // void readMbus();

  private:
    // Pwm pwm;
    // Kamstrup303WA02 kamstrup;
    // bool updateRequested { false };
    // bool mbusEnabled { true };

    // static void read_mbus_task_loop(void* params);
    // static esp_err_t initializeAndEnablePwm(Pwm* pwm);
};

} //namespace heatmeter_mbus
} //namespace esphome