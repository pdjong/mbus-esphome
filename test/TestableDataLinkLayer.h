#include <test_includes.h>
#include <Kamstrup303WA02.h>
#include <vector>
#include <queue>

using std::vector;
using std::queue;

class FakeUartInterface : public esphome::heatmeter_mbus::UartInterface {
  public:
    typedef struct WrittenArray {
      const uint8_t* data;
      size_t len;
    } WrittenArray;

    // UartInterface
    virtual bool read_byte(uint8_t* data) {
      if (0 == this->fake_data_to_return_.size()) {
        return false;
      } else {
        *data = this->fake_data_to_return_.front();
        this->fake_data_to_return_.pop();
        return true;
      }
    }

    virtual bool read_array(uint8_t* data, size_t len) {
      for (size_t i = 0; i < len; ++i) {
        if (this->fake_data_to_return_.empty()) {
          break;
        } else {
          *data++ = this->fake_data_to_return_.front();
          this->fake_data_to_return_.pop();
        }
      }
      return true;
    }

    virtual bool write_array(const uint8_t* data, size_t len) {
      WrittenArray writtenArray = { .data = data, .len = len };
      this->written_arrays_.push_back(writtenArray);
      this->is_write_array_called_ = true;
      return true;
    }

    virtual int available() const {
      return this->fake_data_to_return_.size();
    }

    void flush() {}

    // Access / configure methods
    vector<WrittenArray> written_arrays_;

    void set_fake_data_to_return(uint8_t* fake_data, size_t len) {
      for (size_t i = 0; i < len; ++i) {
        this->fake_data_to_return_.push(fake_data[i]);
      }
    }

    bool is_write_array_called() const {
      return this->is_write_array_called_;
    }

  protected:
    queue<uint8_t> fake_data_to_return_;
    bool is_write_array_called_ { false };
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