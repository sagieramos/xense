#include "utils.h"

XenseStatus serialize_xense_station(const Xense_Station &station,
                                    uint8_t *buffer, size_t *buffer_size,
                                    uint8_t type_id, uint8_t *mac_address,
                                    xense_callback callback) {
  return serialize_xense_message({(void *)&station, Xense_Station_fields},
                                 buffer, buffer_size, type_id, mac_address,
                                 callback);
}
XenseStatus deserialize_xense_station(const uint8_t *buffer, size_t buffer_size,
                                      Xense_Station &station,
                                      xense_callback callback) {
  return deserialize_xense_message({(void *)&station, Xense_Station_fields},
                                   buffer, buffer_size, callback);
}

int main() {
  Xense_Station station = Xense_Station_init_zero;
  strncpy(station.ssid, "MySSID", sizeof(station.ssid));
  station.bssid.size = 6;
  memcpy(station.bssid.bytes, "\x00\x11\x22\x33\x44\x55", 6);
  strncpy(station.password, "MyPassword", sizeof(station.password));
  station.bssid_set = true;

  uint8_t buffer[64];
  size_t message_length;
  pb_ostream_t stream;

  // Serialization
  stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, &Xense_Station_msg, &station)) {
    printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    return 1;
  }
  message_length = stream.bytes_written;
  printf("Serialized length: %zu\n", message_length);
  print_buffer(buffer, message_length);

  // Deserialization
  Xense_Station decoded_station = Xense_Station_init_zero;
  pb_istream_t istream = pb_istream_from_buffer(buffer, message_length);
  if (!pb_decode(&istream, &Xense_Station_msg, &decoded_station)) {
    printf("Decoding failed: %s\n", PB_GET_ERROR(&istream));
    return 1;
  }
  printf("Decoded SSID: %s\n", decoded_station.ssid);
  printf("Decoded BSSID: ");
  for (int i = 0; i < decoded_station.bssid.size; i++) {
    printf("%02x:", decoded_station.bssid.bytes[i]);
  }
  printf("\n");
  printf("Decoded Password: %s\n", decoded_station.password);
  printf("Decoded BSSID Set: %d\n", decoded_station.bssid_set);

  return 0;
}