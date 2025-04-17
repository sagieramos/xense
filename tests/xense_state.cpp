#include "utils.h"

void print_xense_state(const Xense_state &state) {
  std::cout << "Uptime: " << state.uptime << " seconds" << std::endl;
  std::cout << "RSSI: " << state.rssi << " dBm" << std::endl;
  std::cout << "Battery type: " << state.battery_type << std::endl;
  std::cout << "Error code: " << state.error_code << std::endl;
}

void test_xense_state_serialization_deserialization() {
  Xense_state state = Xense_state_init_default;
  state.uptime = 3600;    // 1 hour
  state.rssi = -70;       // Signal strength in dBm
  state.battery_type = 1; // Example battery type
  state.error_code = 0;   // No error

  uint8_t buffer[64];
  size_t buffer_size = sizeof(buffer);

  // Print data before serialization
  std::cout << "State before serialization:" << std::endl;
  print_xense_state(state);

  std::cout << "Starting serialization..." << std::endl;

  // Serialize
  XenseStatus status =
      serialize_xense_state(state, buffer, &buffer_size, XENSE_STATE_TYPE_ID);
  std::cout << "Serialization status: " << status << std::endl;
  assert(status == XENSE_OK);
  assert(buffer_size > 0);
  std::cout << "Buffer size after serialization: " << buffer_size << std::endl;

  // Print serialized data (buffer)
  print_buffer(buffer, buffer_size);

  std::cout << "Starting deserialization..." << std::endl;

  // Deserialize
  Xense_state deserialized_state = Xense_state_init_default;
  status =
      deserialize_xense_state(buffer, buffer_size, deserialized_state, nullptr);
  std::cout << "Deserialization status: " << status << std::endl;
  assert(status == XENSE_OK);
  std::cout << "Checking values..." << std::endl;
  // Check values
  assert(deserialized_state.uptime == state.uptime);
  assert(deserialized_state.rssi == state.rssi);
  assert(deserialized_state.battery_type == state.battery_type);
  assert(deserialized_state.error_code == state.error_code);
  std::cout << "All values match!" << std::endl;
  // Print deserialized values
  std::cout << "Print deserialized values:" << std::endl;
  print_xense_state(deserialized_state);
  std::cout << "Xense state serialization and deserialization test passed!"
            << std::endl;
}

int main() {
  test_xense_state_serialization_deserialization();
  return 0;
}