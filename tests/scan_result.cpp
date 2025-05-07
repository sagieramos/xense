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
  uint8_t test_mac_address[] = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};

  serialize_scan_wifi_list(scan, buffer, &message_length, XENSE_DATA_TYPE_ID,
                           test_mac_address, nullptr);

  uint8_t extracted_mac[6];
  get_mac_address_from_npb(buffer, message_length, extracted_mac);
  print_mac(extracted_mac);

  print_buffer(buffer, message_length);

  // Deserialization
  Xense_ScanResult decoded_scan = Xense_ScanResult_init_zero;
  deserialize_scan_wifi_list(buffer, message_length, decoded_scan, nullptr);

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