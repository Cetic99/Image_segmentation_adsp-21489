# Image segmentation using edge detection algorithm on ADSP-21489 development board

This repository contains codes written in python using OpenCV, code written in Natural C that can be run on PC, and code written for ADSP-21489 DSP.
## Overview
- Image must be of .bmp format.
- Image is read, and converted to grayscale
- Edge detection is done using 2D convolution and Sobel kernel
- Segmentation is done using Connected Component algorithm(some version of it)
- Painting is done using random generator

## How to run
### Python and Natural C
- Terminal
### DSP
- CrossCore Embedded Studio(Not recomended. If you can don't use this tool at all)
