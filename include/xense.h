#ifndef XENSE_H
#define XENSE_H

#include "../external/nanopb/pb.h"
#include "../external/nanopb/pb_common.h"
#include "../external/nanopb/pb_decode.h"
#include "../external/nanopb/pb_encode.h"
#include "../proto/xense/xense.pb.h"

typedef void (*xense_data_callback)(const void *data, size_t len);

bool serialize_xense_data(const Xense_data &data, uint8_t *buffer,
                          size_t *buffer_size, uint8_t type_id,
                          xense_data_callback callback = NULL);

#endif // XENSE_H