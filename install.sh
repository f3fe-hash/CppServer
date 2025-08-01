#!/bin/bash

# This script is for compiling it on a fresh Linux Kernel to install everything needed for the server to run

# Install core libraries for compiling
sudo apt install -y gcc g++ make cmake git

# OpenSSL
sudo apt install -y libssl-dev

# nlohmann
sudo apt-get install nlohmann-json3-dev

# Compile it
make