#include "../lib/xense/xense.h"
#include <cassert>
#include <cstring>
#include <iostream>

void print_buffer(const uint8_t *buffer, size_t buffer_size);
void print_mac_address_from_buffer(const uint8_t *buffer, size_t buffer_size);
void print_mac(const uint8_t *mac);
