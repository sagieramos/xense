#!/bin/bash

# Generate 16 random bytes in hex
hex=$(openssl rand -hex 16)

echo "Generated LMK (hex): $hex"
echo -n "C-style byte array: "
echo "$hex" | awk '{ for(i=1;i<=length;i+=2) printf "0x%s, ", substr($0,i,2); print "" }'
