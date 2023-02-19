/*
 * sobel_edge_detection.h
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#ifndef SOBEL_EDGE_DETECTION_H_
#define SOBEL_EDGE_DETECTION_H_

#include"convolution.h"

/**
 * @brief Edge detection using Sobel kernel
 *
 * @param pixels Input Array of pixels
 * @param out_pixels Output Array of pixels
 * @param width Width of image
 * @param height Height of image
 */
void sobel_edge_detector(byte *pixels, byte **out_pixels, uint32 width,
		uint32 height);

#endif /* SOBEL_EDGE_DETECTION_H_ */
