#include "./npb_transcode.h"

#define TYPE_ID 1
#define MAC_ADDRESS 6
#define TABLE_SIZE 32

NpbStatus npb_encode(const NpbMessageDescriptor &desc, uint8_t *buffer,
                     size_t *buffer_size, uint8_t type_id, uint8_t *mac_address,
                     xense_callback callback) {
  if (!buffer || !buffer_size || !desc.serialized_message || !desc.fields ||
      !mac_address)
    return NPB_NULL_BUFFER;

  if (*buffer_size < TYPE_ID + MAC_ADDRESS)
    return NPB_INVALID_SIZE;

  buffer[0] = type_id;
  memcpy(buffer + TYPE_ID, mac_address, MAC_ADDRESS);

  pb_ostream_t stream = pb_ostream_from_buffer(
      buffer + TYPE_ID + MAC_ADDRESS, *buffer_size - TYPE_ID - MAC_ADDRESS);
  if (!pb_encode(&stream, desc.fields, desc.serialized_message))
    return NPB_ENCODE_FAILED;

  size_t message_size = TYPE_ID + MAC_ADDRESS + stream.bytes_written;

  *buffer_size = message_size;

  if (callback)
    callback(buffer, *buffer_size);

  return NPB_OK;
}

NpbStatus npb_decode(const NpbMessageDescriptor &desc, const uint8_t *buffer,
                     size_t buffer_size, xense_callback callback) {
  if (!buffer || buffer_size < TYPE_ID + MAC_ADDRESS ||
      !desc.deserialized_message || !desc.fields)
    return NPB_INVALID_SIZE;

  pb_istream_t stream = pb_istream_from_buffer(
      buffer + TYPE_ID + MAC_ADDRESS, buffer_size - TYPE_ID - MAC_ADDRESS);
  if (!pb_decode(&stream, desc.fields, desc.deserialized_message))
    return NPB_DECODE_FAILED;

  if (callback)
    callback(buffer, buffer_size);

  return NPB_OK;
}

NpbStatus get_mac_address_from_npb(const uint8_t *buffer, size_t buffer_size,
                                   uint8_t *mac_address) {
  if (!buffer || buffer_size < TYPE_ID + MAC_ADDRESS || !mac_address)
    return NPB_NULL_BUFFER;

  memcpy(mac_address, buffer + TYPE_ID, MAC_ADDRESS);
  return NPB_OK;
}

typedef struct {
    uint8_t mac[6];
    SlotState state;
} MacEntry;

MacEntry mac_table[TABLE_SIZE];

uint32_t mac_hash(const uint8_t *mac) {
    uint32_t hash = 0;
    for (int i = 0; i < 6; i++) {
        hash = (hash * 31) ^ mac[i];
    }
    return hash % TABLE_SIZE;
}

bool mac_exists(const uint8_t *mac) {
    uint32_t index = mac_hash(mac);
    for (int i = 0; i < TABLE_SIZE; i++) {
        uint32_t probe = (index + i) % TABLE_SIZE;
        if (mac_table[probe].state == SLOT_EMPTY) return false;
        if (mac_table[probe].state == SLOT_OCCUPIED &&
            memcmp(mac_table[probe].mac, mac, 6) == 0) {
            return true;
        }
    }
    return false;
}

bool mac_insert(const uint8_t *mac) {
    if (mac_exists(mac)) return false;

    uint32_t index = mac_hash(mac);
    for (int i = 0; i < TABLE_SIZE; i++) {
        uint32_t probe = (index + i) % TABLE_SIZE;
        if (mac_table[probe].state == SLOT_EMPTY || mac_table[probe].state == SLOT_TOMBSTONE) {
            memcpy(mac_table[probe].mac, mac, 6);
            mac_table[probe].state = SLOT_OCCUPIED;
            return true;
        }
    }
    return false; // Table full
}

bool mac_delete(const uint8_t *mac) {
    uint32_t index = mac_hash(mac);
    for (int i = 0; i < TABLE_SIZE; i++) {
        uint32_t probe = (index + i) % TABLE_SIZE;
        if (mac_table[probe].state == SLOT_EMPTY) return false; // Not found
        if (mac_table[probe].state == SLOT_OCCUPIED &&
            memcmp(mac_table[probe].mac, mac, 6) == 0) {
            mac_table[probe].state = SLOT_TOMBSTONE;
            return true; // Deleted
        }
    }
    return false; // Not found
}

