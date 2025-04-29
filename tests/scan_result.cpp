#include "utils.h"

int main() {
  Xense_ScanResult scan = Xense_ScanResult_init_zero;
  scan.access_points_count = 2;

  strncpy(scan.access_points[0].ssid, "Net1",
          sizeof(scan.access_points[0].ssid));
  scan.access_points[0].bssid.size = 6;
  memcpy(scan.access_points[0].bssid.bytes, "\x00\x11\x22\x33\x44\x55", 6);
  scan.access_points[0].rssi = -45;
  scan.access_points[0].authmode = 3;
  scan.access_points[0].channel = 6;

  strncpy(scan.access_points[1].ssid, "Net2",
          sizeof(scan.access_points[1].ssid));
  scan.access_points[1].bssid.size = 6;
  memcpy(scan.access_points[1].bssid.bytes, "\xaa\xbb\xcc\xdd\xee\xff", 6);
  scan.access_points[1].rssi = -50;
  scan.access_points[1].authmode = 4;
  scan.access_points[1].channel = 11;

  uint8_t buffer[128];
  size_t message_length;
  pb_ostream_t stream;

  // Serialization
  stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, &Xense_ScanResult_msg, &scan)) {
    printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
    return 1;
  }
  message_length = stream.bytes_written;

  // Print the serialized buffer
  print_buffer(buffer, message_length);

  // Deserialization
  Xense_ScanResult decoded_scan = Xense_ScanResult_init_zero;
  pb_istream_t istream = pb_istream_from_buffer(buffer, message_length);
  if (!pb_decode(&istream, &Xense_ScanResult_msg, &decoded_scan)) {
    printf("Decoding failed: %s\n", PB_GET_ERROR(&istream));
    return 1;
  }

  // Print the decoded values
  for (size_t i = 0; i < decoded_scan.access_points_count; ++i) {
    printf("SSID: %s\n", decoded_scan.access_points[i].ssid);
    printf("BSSID: ");
    for (int j = 0; j < decoded_scan.access_points[i].bssid.size; j++) {
      printf("%02x:", decoded_scan.access_points[i].bssid.bytes[j]);
    }
    printf("\n");
    printf("RSSI: %d\n", decoded_scan.access_points[i].rssi);
    printf("Authmode: %d\n", decoded_scan.access_points[i].authmode);
    printf("Channel: %d\n", decoded_scan.access_points[i].channel);
  }

  return 0;
}