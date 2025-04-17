#include "utils.h"

// Utility function to print a buffer as hex values
void print_buffer(const uint8_t *buffer, size_t buffer_size)
{
    std::cout << "Buffer content: ";
    for (size_t i = 0; i < buffer_size; ++i)
    {
        std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::dec << std::endl; // Switch back to decimal format
}