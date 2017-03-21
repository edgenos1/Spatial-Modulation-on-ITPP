# Spatial-Modulation-on-ITPP
This program simulates spatial modulation and spatial multiplexing by transmitting image file(ex .bmp file).

![Example](http://i.imgur.com/QmCPDIg.png, "Example")

# Requirements
- CMake 3.7.1
- g++ 6.3.0
- OpenCV 3.2.0
- IT++ 4.3.1

# Build
```bash
mkdir build && cd build
cmake ..
make
```
# Usage
* 4x4 QPSK Spatial Modulation at 10dB
```
./simulator 4 4 4 10 sm lena.bmp output.csv
```
* 2x8 8PSK Spatial Multiplexing at 0dB
```
./simulator 2 8 8 0 smx lena.bmp output.csv
```
