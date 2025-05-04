#include "./xense.h"

NpbStatus serialize_scan_wifi_list(const Xense_ScanResult aps, uint8_t *buffer,
                                   size_t *buffer_size, uint8_t type_id,
                                   uint8_t *mac_address,
                                   xense_callback callback) {
  return npb_encode({&aps, Xense_ScanResult_fields}, buffer, buffer_size,
                    type_id, mac_address, callback);
}

NpbStatus deserialize_scan_wifi_list(const uint8_t *buffer, size_t buffer_size,
                                     Xense_ScanResult &data,
                                     xense_callback callback) {
  return npb_decode({&data, Xense_ScanResult_fields}, buffer, buffer_size,
                    callback);
}