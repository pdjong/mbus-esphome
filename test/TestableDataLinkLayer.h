#include "Kamstrup303WA02.h"

class FakeUartInterface : public esphome::heatmeter_mbus::UartInterface {
  public:
    virtual bool read_byte(uint8_t* data) {
      return true;
    }

    virtual bool read_array(uint8_t* data, size_t len) {
      return true;
    }

    virtual bool write_array(uint8_t* data, size_t len) {
      return true;
    }

    virtual int available() const {
      return 0;
    }

    void flush() {}
};

class TestableDataLinkLayer : public esphome::heatmeter_mbus::Kamstrup303WA02::DataLinkLayer {
  public:
    TestableDataLinkLayer(FakeUartInterface* uartInterface) : esphome::heatmeter_mbus::Kamstrup303WA02::DataLinkLayer(uartInterface) {}

    uint8_t call_calculate_checksum(const uint8_t* data, size_t length) {
      return this->calculate_checksum(data, length);
    }
};