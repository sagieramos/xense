#!/bin/bash

# This script generates a QR code for Wi-Fi credentials using Python and the qrcode library.
# It creates a virtual environment, installs the required package, and runs a Python script to generate the QR code.
# Usage: ./generate_wifi_qr.sh <SSID> <PASSWORD>
# Use https://qifi.org/ to generate the QR code if you don't want to use this script.

# Name of the virtual environment directory
VENV_DIR="wifi_qr_env"

# Create a virtual environment if it doesn't exist
if [ ! -d "$VENV_DIR" ]; then
    echo "Creating virtual environment..."
    python3 -m venv $VENV_DIR
else
    echo "Virtual environment already exists."
fi

# Activate the virtual environment
echo "Activating virtual environment..."
source $VENV_DIR/bin/activate

# Install necessary Python package (qrcode)
echo "Installing required Python packages..."
pip install --quiet qrcode[pil]

# Python script to generate the Wi-Fi QR code
cat << 'EOF' > wifi_qr.py
import qrcode
import sys

def generate_wifi_qr(ssid, password):
    wifi_string = f"WIFI:T:WPA;S:{ssid};P:{password};;"

    qr = qrcode.QRCode(
        version=1,
        error_correction=qrcode.constants.ERROR_CORRECT_L,
        box_size=2,
        border=2,
    )
    qr.add_data(wifi_string)
    qr.make(fit=True)

    qr.print_ascii(invert=True)  # Nice looking in terminal

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python wifi_qr.py <SSID> <PASSWORD>")
    else:
        generate_wifi_qr(sys.argv[1], sys.argv[2])
EOF

# Run the Python script with arguments passed to the shell script
echo "Running Wi-Fi QR generation script..."
python wifi_qr.py "$1" "$2"

# Deactivate the virtual environment
echo "Deactivating virtual environment..."
deactivate
