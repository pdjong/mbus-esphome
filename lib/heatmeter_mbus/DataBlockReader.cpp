#ifdef UNIT_TEST
#include <test_includes.h>
#endif // UNIT_TEST

#include "DataBlockReader.h"

using std::vector;

namespace esphome {
namespace warmtemetermbus {

vector<Kamstrup303WA02::DataBlock*>* DataBlockReader::read_data_blocks_from_long_frame(Kamstrup303WA02::DataLinkLayer::LongFrame* long_frame) {
  return nullptr;
}
} //namespace warmtemetermbus
} //namespace esphome