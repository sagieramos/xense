#!/bin/bash

VENV_PATH="."

# Step 1: Create a virtual environment
python3 -m venv $VENV_PATH

# Step 2: Activate the virtual environment
source $VENV_PATH/bin/activate

# Step 3: Install required packages within the virtual environment
pip install protobuf grpcio-tools

# Check if protoc (protobuf-compiler) is installed
if ! command -v protoc &> /dev/null; then
    echo "protoc (protobuf-compiler) is not installed. Installing now..."
    sudo apt update
    sudo apt install -y protobuf-compiler

    # Verify installation success
    if ! command -v protoc &> /dev/null; then
        echo "Failed to install protoc."
        exit 1
    else
        echo "protoc installed successfully."
    fi
else
    echo "protoc is already installed."
fi

# Check if a .proto file was provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 path/to/yourfile.proto [output_directory]"
    exit 1
fi

PROTO_FILE="$1"
PROTO_DIR=$(dirname "$PROTO_FILE")
PROTO_BASENAME=$(basename "$PROTO_FILE" .proto)

# Check if the .proto file exists
if [ ! -f "$PROTO_FILE" ]; then
    echo "Error: .proto file '$PROTO_FILE' not found."
    exit 1
fi

# Set default output directory
DEFAULT_OUTPUT_DIR="generated"
# Check if an output directory was provided as the second argument
if [ -n "$2" ]; then
    OUTPUT_DIR="$2"
else
    OUTPUT_DIR="$DEFAULT_OUTPUT_DIR"
fi

# Create the output directory if it does not exist
mkdir -p "$OUTPUT_DIR"

# Define the nanopb directory
NANOPB_DIR="external/nanopb"

# Check if nanopb directory exists
if [ ! -d "$NANOPB_DIR" ]; then
    echo "Error: nanopb directory not found at $NANOPB_DIR."
    exit 1
fi

# Define paths for nanopb plugin and proto
NANOPB_PLUGIN="$NANOPB_DIR/generator/protoc-gen-nanopb"
NANOPB_PROTO_DIR="$NANOPB_DIR/generator/proto"

# Check if protoc-gen-nanopb exists and is executable
if [ ! -f "$NANOPB_PLUGIN" ]; then
    echo "Error: protoc-gen-nanopb not found at $NANOPB_PLUGIN."
    exit 1
fi
if [ ! -x "$NANOPB_PLUGIN" ]; then
    echo "Making protoc-gen-nanopb executable..."
    chmod +x "$NANOPB_PLUGIN"
fi

# Check if nanopb.proto exists
if [ ! -f "$NANOPB_PROTO_DIR/nanopb.proto" ]; then
    echo "Error: nanopb.proto not found at $NANOPB_PROTO_DIR/nanopb.proto."
    exit 1
fi

# Run the protoc command with the specified plugin and output directory
protoc \
  --proto_path="$PROTO_DIR" \
  --proto_path=. \
  --proto_path="$NANOPB_PROTO_DIR" \
  --plugin=protoc-gen-nanopb="$NANOPB_PLUGIN" \
  --nanopb_out="$OUTPUT_DIR" \
  "$PROTO_FILE" 2> protoc_error.log

if [ $? -eq 0 ]; then
    echo "Protobuf files generated successfully in $OUTPUT_DIR."
else
    echo "Failed to generate protobuf files. Check protoc_error.log for details:"
    cat protoc_error.log
    exit 1
fi

deactivate
