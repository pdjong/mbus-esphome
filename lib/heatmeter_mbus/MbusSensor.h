#ifndef MBUSSENSOR_H_
#define MBUSSENSOR_H_

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "Kamstrup303WA02.h"

namespace esphome {
namespace warmtemetermbus {

class MbusSensor : public sensor::Sensor {
  public:
    MbusSensor(uint8_t index) : index_(index) {}

  //protected:
    uint8_t index_;
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // MBUSSENSOR_H_