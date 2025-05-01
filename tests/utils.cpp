#include "utils.h"

// Utility function to print a buffer as hex values
void print_buffer(const uint8_t *buffer, size_t buffer_size) {
  std::cout << "Buffer content: ";
  for (size_t i = 0; i < buffer_size; ++i) {
    std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
  }
  std::cout << std::dec << std::endl; // Switch back to decimal format
}

void print_mac_address_from_buffer(const uint8_t *buffer, size_t buffer_size) {
  if (!buffer || buffer_size < 6) {
    printf("Invalid buffer or too small to contain a MAC address.\n");
    return;
  }

  const uint8_t *mac = buffer + buffer_size - 6;
  printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2],
         mac[3], mac[4], mac[5]);
}
