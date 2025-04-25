#include "utils.h"

uint8_t buffer[128];
size_t message_length;
pb_ostream_t stream;

int main() {
  Xense_Station station = Xense_Station_init_zero;
  /*   station.which_target = Xense_Station_ssid_tag;
    strncpy(station.target.ssid, "MyWiFi", sizeof(station.target.ssid)); */

  station.which_target = Xense_Station_bssid_tag;
  station.target.bssid.size = 6; // BSSID size is 6 bytes
  memcpy(station.target.bssid.bytes, "\xaa\xbb\xcc\xdd\xee\xff", 6);

  strncpy(station.password, "MyPassword123", sizeof(station.password));

  // Serialization
  stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, &Xense_Station_msg, &station)) {
    printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
  }
  message_length = stream.bytes_written;
  print_buffer(buffer, message_length);

  Xense_Station decoded_station = Xense_Station_init_zero;
  pb_istream_t istream = pb_istream_from_buffer(buffer, message_length);
  if (!pb_decode(&istream, &Xense_Station_msg, &decoded_station)) {
    printf("Decoding failed: %s\n", PB_GET_ERROR(&istream));
  }
  if (decoded_station.which_target == Xense_Station_ssid_tag) {
    printf("SSID: %s\n", decoded_station.target.ssid);
  } else if (decoded_station.which_target == Xense_Station_bssid_tag) {
    printf("BSSID: ");
    for (int i = 0; i < decoded_station.target.bssid.size; i++) {
      printf("%02x:", decoded_station.target.bssid.bytes[i]);
    }
    printf("\n");
  }
  printf("Password: %s\n", decoded_station.password);

  return 0;
}
