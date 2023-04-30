#include "esphome/core/log.h"
#include <math.h>
#include "HeatMeterMbus.h"
#include "Kamstrup303WA02.h"

//#include <iostream>
//#include <sstream>

using namespace std; 

namespace esphome
{
  namespace warmtemetermbus
  {

    static const char *TAG = "heatmetermbus.sensor";

    void HeatMeterMbus::setup()
    {
    }

    void HeatMeterMbus::update()
    {
      // Let's request data, and wait for its results :-)
      Kamstrup303WA02::MeterData meterData;
      ESP_LOGI(TAG, "About to readData");
      bool readSuccessful = kamstrup.readData(&meterData);

      if (readSuccessful)
      {
        ESP_LOGI(TAG, "Successfully read meter data");

        // Heat energy E1
        // Source unit can be Wh or J
        // TODO: Check source unit. For now, assume Wh
        // Use tenPower - 3 to convert from Wh to kWh
        float heatEnergyE1Value = pow(10, meterData.heatEnergyE1.tenPower - 3) * meterData.heatEnergyE1.value;
        heat_energy_e1_sensor_->publish_state(heatEnergyE1Value);

        // Volume V1
        // Source unit is always m3
        // Use tenPower + 3 to convert to liters
        float volumeV1Value = pow(10, meterData.volumeV1.tenPower + 3) * meterData.volumeV1.value;
        volume_v1_sensor_->publish_state(volumeV1Value);

        // Energy E8 Inlet
        // Source unit is always m3 * deg Celcius
        float energyE8InletValue = pow(10, meterData.energyE8.tenPower) * meterData.energyE8.value;
        energy_e8_inlet_sensor_->publish_state(energyE8InletValue);

        // Energy E9 Outlet
        // Source unit is always m3 * deg Celcius
        float energyE9OutletValue = pow(10, meterData.energyE9.tenPower) * meterData.energyE9.value;
        energy_e9_outlet_sensor_->publish_state(energyE9OutletValue);
        
        // Operating Hours
        // Source unit can be seconds, minutes, hours or days
        // TODO: Check source unit. For now, assume hours, convert to days
        // No ten power
        ESP_LOGI(TAG, "Operating hours raw value at top level: %d", meterData.operatingHours.value);
        float operatingHoursValueInDays = meterData.operatingHours.value / 24.0;
        operating_hours_sensor_->publish_state(operatingHoursValueInDays);

        // Error Hour Counter
        // Source unit can be seconds, minutes, hours or days
        // TODO: Check source unit. For now, assume hours, convert to days
        // No ten power
        float errorHourCounterValueInDays = meterData.errorHourCounter.value / 24.0;
        error_hour_counter_sensor_->publish_state(errorHourCounterValueInDays);

        // T1 actual
        float t1ActualValue = pow(10, meterData.t1Actual.tenPower) * meterData.t1Actual.value;
        t1_actual_sensor_->publish_state(t1ActualValue);
      }
      else
      {
        ESP_LOGE(TAG, "Did not successfully read meter data");
      }
    }

    float HeatMeterMbus::get_setup_priority() const
    {
      // After UART bus
      return setup_priority::BUS - 1.0f;
    }

    void HeatMeterMbus::dump_config()
    {
      ESP_LOGCONFIG(TAG, "HeatMeterMbus sensor");
      LOG_SENSOR("  ", "T1 Actual", this->t1_actual_sensor_);
      LOG_SENSOR("  ", "Heat Energy E1", this->heat_energy_e1_sensor_);
      LOG_SENSOR("  ", "Volume V1", this->volume_v1_sensor_);
      LOG_SENSOR("  ", "Energy E8 Inlet", this->energy_e8_inlet_sensor_);
      LOG_SENSOR("  ", "Energy E9 Outlet", this->energy_e9_outlet_sensor_);
      LOG_SENSOR("  ", "Operating Hours", this->operating_hours_sensor_);
      LOG_SENSOR("  ", "Error Hour Counter", this->error_hour_counter_sensor_);
    }

  } // namespace warmtemetermbus
} // namespace esphome