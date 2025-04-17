#include "utils.h"

void print_metadata(const Xense_metadata &metadata) {
  std::cout << "Firmware version: " << metadata.firmware_version << std::endl;
  std::cout << "Hardware revision: " << metadata.hardware_revision << std::endl;
  std::cout << "Serial number: " << metadata.serial_number << std::endl;
  std::cout << "Manufacturer ID: " << metadata.manufacturer_id << std::endl;
  std::cout << "Model ID: " << metadata.model_id << std::endl;
}

void test_xense_metadata_serialization_deserialization() {
  Xense_metadata metadata = Xense_metadata_init_default;
  metadata.firmware_version = 1.0f;
  metadata.hardware_revision = 1.0f;
  metadata.serial_number = 123456789;
  metadata.manufacturer_id = 987654321;
  metadata.model_id = 123456;

  uint8_t buffer[64];
  size_t buffer_size = sizeof(buffer);

  // Print data before serialization
  std::cout << "Metadata before serialization:" << std::endl;
  print_metadata(metadata);

  std::cout << "Starting serialization..." << std::endl;

  // Serialize
  XenseStatus status = serialize_xense_metadata(metadata, buffer, &buffer_size,
                                                XENSE_METADATA_TYPE_ID);
  std::cout << "Serialization status: " << status << std::endl;
  assert(status == XENSE_OK);
  assert(buffer_size > 0);
  std::cout << "Buffer size after serialization: " << buffer_size << std::endl;

  // Print serialized data (buffer)
  print_buffer(buffer, buffer_size);

  std::cout << "Starting deserialization..." << std::endl;

  // Deserialize
  Xense_metadata deserialized_metadata = Xense_metadata_init_default;
  status = deserialize_xense_metadata(buffer, buffer_size,
                                      deserialized_metadata, nullptr);
  std::cout << "Deserialization status: " << status << std::endl;
  assert(status == XENSE_OK);

  std::cout << "Checking values..." << std::endl;

  // Check values
  assert(deserialized_metadata.firmware_version == metadata.firmware_version);
  assert(deserialized_metadata.hardware_revision == metadata.hardware_revision);
  assert(deserialized_metadata.serial_number == metadata.serial_number);
  assert(deserialized_metadata.manufacturer_id == metadata.manufacturer_id);
  assert(deserialized_metadata.model_id == metadata.model_id);
  std::cout << "All values match!" << std::endl;
  // Print deserialized values
  std::cout << "Print deserialized values:" << std::endl;
  print_metadata(deserialized_metadata);
  std::cout << "Xense metadata serialization and deserialization test passed!"
            << std::endl;
}

int main() {
  test_xense_metadata_serialization_deserialization();
  return 0;
}