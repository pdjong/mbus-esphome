#ifndef UNIT_TEST

#include "esphome/core/log.h"
#include "MbusSensor.h"
#include <math.h>

using namespace std; 

namespace esphome
{
namespace warmtemetermbus
{

static const char *TAG = "MbusSensor";

void MbusSensor::transform_and_publish(Kamstrup303WA02::DataBlock* data_block) {
  switch (data_block->data_length) {
    case 2: {
      int16_t *raw_value = reinterpret_cast<int16_t*>(data_block->binary_data);
      float value = static_cast<float>(*raw_value * pow(10, data_block->ten_power));
      this->publish_state(value);
      break;
    }
    case 4: {
      int32_t *raw_value = reinterpret_cast<int32_t*>(data_block->binary_data);
      float value = static_cast<float>(*raw_value * pow(10, data_block->ten_power));
      this->publish_state(value);
      break;
    }
    default:
      break;
  }
}

} // namespace warmtemetermbus
} // namespace esphome

#endif // UNIT_TEST
