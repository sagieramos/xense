#include "utils.h"

void print_gps_data(const Xense_gps &data) {
  std::cout << "Latitude: " << data.lat_deg << "." << data.lat_billionths
            << " degrees" << std::endl;
  std::cout << "Longitude: " << data.lon_deg << "." << data.lon_billionths
            << " degrees" << std::endl;
  std::cout << "Altitude: " << data.altitude << " meters" << std::endl;
  std::cout << "Satellites: " << data.satellites << std::endl;
  std::cout << "Date: " << data.date << std::endl;
  std::cout << "Time: " << data.time << std::endl;
  std::cout << "HDOP: " << data.hdop << std::endl;
}

void test_xense_gps_serialization_deserialization() {
  Xense_gps gps_data = Xense_gps_init_default;
  gps_data.lat_deg = 6;
  gps_data.lat_billionths = 987654321;
  gps_data.lon_deg = 3;
  gps_data.lon_billionths = 123456789;
  gps_data.altitude = 1000;
  gps_data.satellites = 5;
  gps_data.date = 20231010; // YYYYMMDD
  gps_data.time = 123456;   // HHMMSS
  gps_data.hdop = 0.5f;

  uint8_t buffer[64];
  size_t buffer_size = sizeof(buffer);

  // Print data before serialization
  std::cout << "GPS Data before serialization:" << std::endl;
  print_gps_data(gps_data);

  std::cout << "Starting serialization..." << std::endl;

  // Serialize
  XenseStatus status =
      serialize_xense_gps(gps_data, buffer, &buffer_size, XENSE_GPS_TYPE_ID);
  std::cout << "Serialization status: " << status << std::endl;
  assert(status == XENSE_OK);
  assert(buffer_size > 0);
  std::cout << "Buffer size after serialization: " << buffer_size << std::endl;

  // Print serialized data (buffer)
  print_buffer(buffer, buffer_size);

  std::cout << "Starting deserialization..." << std::endl;

  // Deserialize
  Xense_gps deserialized_gps_data = Xense_gps_init_default;
  status = deserialize_xense_gps(buffer, buffer_size, deserialized_gps_data,
                                 nullptr);
  std::cout << "Deserialization status: " << status << std::endl;
  assert(status == XENSE_OK);

  std::cout << "Checking values..." << std::endl;

  // Check values
  assert(deserialized_gps_data.lat_deg == gps_data.lat_deg);
  assert(deserialized_gps_data.lat_billionths == gps_data.lat_billionths);
  assert(deserialized_gps_data.lon_deg == gps_data.lon_deg);
  assert(deserialized_gps_data.lon_billionths == gps_data.lon_billionths);
  assert(deserialized_gps_data.altitude == gps_data.altitude);
  assert(deserialized_gps_data.satellites == gps_data.satellites);
  assert(deserialized_gps_data.date == gps_data.date);
  assert(deserialized_gps_data.time == gps_data.time);
  assert(deserialized_gps_data.hdop == gps_data.hdop);
  std::cout << "All values match!" << std::endl;
  // Print deserialized values
  std::cout << "Print deserialized values:" << std::endl;
  print_gps_data(deserialized_gps_data);
  std::cout << "Xense GPS serialization and deserialization test passed!"
            << std::endl;
}

int main() {
  test_xense_gps_serialization_deserialization();
  return 0;
}