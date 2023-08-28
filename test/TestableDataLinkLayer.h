#include <test_includes.h>
#include <Kamstrup303WA02.h>
#include <vector>

using std::vector;

class FakeUartInterface : public esphome::heatmeter_mbus::UartInterface {
  public:
    typedef struct WrittenArray {
      uint8_t* data;
      size_t len;
    } WrittenArray;

    virtual bool read_byte(uint8_t* data) {
      return true;
    }

    virtual bool read_array(uint8_t* data, size_t len) {
      return true;
    }

    virtual bool write_array(uint8_t* data, size_t len) {
      WrittenArray writtenArray = { .data = data, .len = len };
      this->written_arrays.push_back(writtenArray);
      return true;
    }

    virtual int available() const {
      return 0;
    }

    void flush() {}

    vector<WrittenArray> written_arrays;
};

class TestableDataLinkLayer : public esphome::heatmeter_mbus::Kamstrup303WA02::DataLinkLayer {
  public:
    TestableDataLinkLayer(FakeUartInterface* uartInterface) : esphome::heatmeter_mbus::Kamstrup303WA02::DataLinkLayer(uartInterface) {}

    bool call_try_send_short_frame(const uint8_t c, const uint8_t a) {
      return this->try_send_short_frame(c, a);
    }

    uint8_t call_calculate_checksum(const uint8_t* data, size_t length) {
      return this->calculate_checksum(data, length);
    }
};