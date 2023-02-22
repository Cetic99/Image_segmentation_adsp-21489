# Image segmentation using edge detection algorithm on ADSP-21489 development board

This repository contains codes written in python using OpenCV, code written in Natural C that can be run on PC, and code written for ADSP-21489 DSP.
## Overview
- Image must be of .bmp format.
- Image is read, and converted to grayscale
- Edge detection is done using 2D convolution and Sobel kernel
- Segmentation is done using Connected Component algorithm(some version of it)
- Painting is done in code or by writting lookup table in bmp header 

## How to run
### Python and Natural C
- Open main.py or main.c in VS Code or some other text editor and change name of image
- Compile
- Run
### DSP
- Clone repository
- Open project in CCES(Cross Core Embedded Studio)
- Chose debug configuration to use(Simulation or EZ-KIT)
- Run debugging

NOTE: Images needs to be in Debug folder
