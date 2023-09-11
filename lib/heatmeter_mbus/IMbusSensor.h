#ifndef IMBUSSENSOR_H_
#define IMBUSSENSOR_H_

#include "Kamstrup303WA02.h"

namespace esphome {
namespace warmtemetermbus {

class IMbusSensor {
  public:
    IMbusSensor(uint8_t index) : index_(index) {}
    
    virtual void transform_and_publish(Kamstrup303WA02::DataBlock* data_block) = 0;

    uint8_t index_;
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // IMBUSSENSOR_H_