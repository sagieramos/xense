#ifndef XENSE_H
#define XENSE_H

#include "../../external/nanopb/pb.h"
#include "../../external/nanopb/pb_common.h"
#include "../../external/nanopb/pb_decode.h"
#include "../../external/nanopb/pb_encode.h"
#include "../npb/npb_transcode.h"
#include "./protoc/wifi.pb.h"
#include "./protoc/xense.pb.h"

/* Type Identifiers (for use in manual encoding/decoding) */
#define XENSE_GPS_TYPE_ID 2
#define XENSE_METADATA_TYPE_ID 3
#define XENSE_STATE_TYPE_ID 4
#define XENSE_DATA_TYPE_ID 5

// xense_common.h

NpbStatus serialize_xense_data(const Xense_data &data, uint8_t *buffer,
                               size_t *buffer_size, uint8_t type_id,
                               uint8_t *mac_address, xense_callback callback);

NpbStatus deserialize_xense_data(const uint8_t *buffer, size_t buffer_size,
                                 Xense_data &data,
                                 xense_callback callback = nullptr);

NpbStatus serialize_xense_gps(const Xense_gps &gps, uint8_t *buffer,
                              size_t *buffer_size, uint8_t type_id,
                              uint8_t *mac_address, xense_callback callback);
NpbStatus deserialize_xense_gps(const uint8_t *buffer, size_t buffer_size,
                                Xense_gps &gps,
                                xense_callback callback = nullptr);

NpbStatus serialize_xense_metadata(const Xense_metadata &metadata,
                                   uint8_t *buffer, size_t *buffer_size,
                                   uint8_t type_id, uint8_t *mac_address,
                                   xense_callback callback);
NpbStatus deserialize_xense_metadata(const uint8_t *buffer, size_t buffer_size,
                                     Xense_metadata &metadata,
                                     xense_callback callback = nullptr);
NpbStatus serialize_xense_state(const Xense_state &state, uint8_t *buffer,
                                size_t *buffer_size, uint8_t type_id,
                                uint8_t *mac_address, xense_callback callback);
NpbStatus deserialize_xense_state(const uint8_t *buffer, size_t buffer_size,
                                  Xense_state &state,
                                  xense_callback callback = nullptr);

NpbStatus get_mac_address_from_xense_buffer(const uint8_t *buffer,
                                            size_t buffer_size,
                                            uint8_t *mac_address);

NpbStatus serialize_scan_wifi_list(const Xense_ScanResult aps, uint8_t *buffer,
                                   size_t *buffer_size, uint8_t type_id,
                                   uint8_t *mac_address,
                                   xense_callback callback);
NpbStatus deserialize_scan_wifi_list(const uint8_t *buffer, size_t buffer_size,
                                     Xense_ScanResult &data,
                                     xense_callback callback);

#endif // XENSE_H