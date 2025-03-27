#!/bin/bash

# Path to the C++ binary

CPP_BINARY="./forward"

# Input and output directories
INPUT_DIR="../inputfiles/"
OUTPUT_DIR="../outputfiles"

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

# Iterate over all .txt files in the input directory
for input_file in "$INPUT_DIR"/*.txt; do
    # Extract the file name without the directory
    filename=$(basename "$input_file" ".txt")
    
    # Construct the output file path
    output_file="$OUTPUT_DIR/$filename"
    output_file+="_forward_out"
    output_file+=".txt"
    
    # Run the binary on the input file and redirect the output to the output file
    "$CPP_BINARY"  "$input_file" 100 > "$output_file"
    
    echo "Processed: $input_file -> $output_file"
done
