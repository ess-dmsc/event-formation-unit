#!/bin/bash

# Parse command line arguments
while getopts "f:d" opt; do
    case $opt in
        f)
            file_path=$OPTARG
            ;;
        d)
            debug_mode=true
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            echo "Usage: $0 -f <file_path> [-d]" >&2
            echo "Options:" >&2
            echo "  -f <file_path>  Path to the file to be processed" >&2
            echo "  -d              Enable debug mode" >&2
            exit 1
            ;;
    esac
done

# Check if the virtual environment exists
if [ ! -d "timepix_gen" ]; then
    # Create a new virtual environment
    python -m venv timepix_gen > /dev/null 2>&1
    source timepix_gen/bin/activate
    echo "Install necessary packages..."
    pip install -r requirements.txt > /dev/null 2>&1
fi

# Define a function to be executed on exit
cleanup() {
    # Add your cleanup code here
    echo "Script is exiting and cleaning up! wait!"

    echo "Removed the virtual environment"
    # Deactivate the virtual environment
    deactivate

    echo "delete venv folder"
    rm -Rf timepix_gen
}

# Set the trap to execute the cleanup function on exit
trap cleanup EXIT

echo "Start data generation..."
# Activate the virtual environment
source timepix_gen/bin/activate

# Run your Python script(s) here, passing the file path as an argument
if [ "$debug_mode" = true ]; then
    python tpx_generator.py -f "$file_path" -d
else
    python tpx_generator.py -f "$file_path"
fi