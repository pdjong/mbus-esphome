#ifndef DATABLOCKREADER_H_
#define DATABLOCKREADER_H_

#ifndef UNIT_TEST
#include "esphome/core/datatypes.h"
#endif // UNIT_TEST

#include <vector>
#include "Kamstrup303WA02.h"

namespace esphome {
namespace warmtemetermbus {

class DataBlockReader {
  public:
    std::vector<Kamstrup303WA02::DataBlock*>* read_data_blocks_from_long_frame(Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame);

  protected:
     Kamstrup303WA02::DataBlock* read_next_data_block();
};

} //namespace warmtemetermbus
} //namespace esphome

#endif // DATABLOCKREADER_H_