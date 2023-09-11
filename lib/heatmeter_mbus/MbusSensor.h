#ifndef MBUSSENSOR_H_
#define MBUSSENSOR_H_

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "Kamstrup303WA02.h"
#include "IMbusSensor.h"

namespace esphome {
namespace warmtemetermbus {

class MbusSensor : public sensor::Sensor, public IMbusSensor {
  public:
    MbusSensor(uint8_t index) : IMbusSensor(index) {}

    virtual void transform_and_publish(Kamstrup303WA02::DataBlock* data_block);
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // MBUSSENSOR_H_