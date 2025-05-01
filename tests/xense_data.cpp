#include "utils.h"

// Utility function to print the Xense_data structure
void print_data(const Xense_data &data) {
  std::cout << "Battery capacity: " << data.battery_capacity << std::endl;
  std::cout << "Absolute load power: " << data.absolute_load_power << std::endl;
  std::cout << "Load power: " << data.load_power << std::endl;
  std::cout << "Battery status: " << data.battery_status << std::endl;
  std::cout << "Battery voltage: " << data.battery_voltage << std::endl;
  std::cout << "Inverter power: " << data.inverter_power << std::endl;
  std::cout << "Is charging: " << (data.is_charging ? "true" : "false")
            << std::endl;
}

uint8_t test_mac_address[] = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};

void test_xense_data_serialization_deserialization() {
  Xense_data data = Xense_data_init_default;
  data.battery_capacity = 75.0f;
  data.absolute_load_power = 100.0f;
  data.load_power = 50.0f;
  data.battery_status = 80.0f;
  data.battery_voltage = 12.5f;
  data.inverter_power = 200.0f;
  data.is_charging = true;

  uint8_t buffer[64];
  size_t buffer_size = sizeof(buffer);

  // Print data before serialization
  std::cout << "Data before serialization:" << std::endl;
  print_data(data);

  std::cout << "Starting serialization..." << std::endl;

  // Serialize
  NpbStatus status =
      serialize_xense_data(data, buffer, &buffer_size, XENSE_DATA_TYPE_ID,
                           test_mac_address, nullptr);
  std::cout << "Serialization status: " << status << std::endl;
  assert(status == NPB_OK);
  assert(buffer_size > 0);
  std::cout << "Buffer size after serialization: " << buffer_size << std::endl;

  // Print serialized data (buffer)
  print_buffer(buffer, buffer_size);
  print_mac_address_from_buffer(buffer, buffer_size);

  std::cout << "Starting deserialization..." << std::endl;

  // Deserialize
  Xense_data deserialized_data = Xense_data_init_default;
  status =
      deserialize_xense_data(buffer, buffer_size, deserialized_data, nullptr);
  std::cout << "Deserialization status: " << status << std::endl;
  assert(status == NPB_OK);

  std::cout << "Checking values..." << std::endl;

  // Check values
  assert(deserialized_data.battery_capacity == data.battery_capacity);
  assert(deserialized_data.absolute_load_power == data.absolute_load_power);
  assert(deserialized_data.load_power == data.load_power);
  assert(deserialized_data.battery_status == data.battery_status);
  assert(deserialized_data.battery_voltage == data.battery_voltage);
  assert(deserialized_data.inverter_power == data.inverter_power);
  assert(deserialized_data.is_charging == data.is_charging);
  std::cout << "All values match!" << std::endl;
  // Print deserialized values
  std::cout << "Print deserialized values:" << std::endl;
  print_data(deserialized_data);
  std::cout << "Xense data serialization and deserialization test passed!"
            << std::endl;
}

int main() {
  test_xense_data_serialization_deserialization();
  return 0;
}
