#include <test_includes.h>
#include <Kamstrup303WA02.h>
#include <vector>
#include <queue>
#include "../test_data_link_layer/TestableDataLinkLayer.h"

using std::vector;
using std::queue;

class TestableKamstrup303WA02 : public esphome::warmtemetermbus::Kamstrup303WA02 {
  public:
    TestableKamstrup303WA02(FakeUartInterface* uart_interface) : esphome::warmtemetermbus::Kamstrup303WA02(uart_interface) {}

    void call_read_next_data_block(DataBlock* data_block) {
      return this->read_next_data_block(data_block);
    }
};