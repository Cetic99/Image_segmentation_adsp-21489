/*
 * normalization.c
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#include"Segmentacija_slike.h"
#include "normalization.h"


#ifdef NORMALIZATION_NO_OPT
/**
 * @brief Normalizing array to 0-255, no optimization
 *
 * @param pixels Array of pixels to be normalized
 * @param width Width of image
 * @param height Height of image
 */
void min_max_normalization(uint32 *pixels, uint32 width, uint32 height) {
	uint32 min = INT_MAX, max = 0, i;
	uint32 length = height * width;
	for (i = 0; i < length; i++) {
		if (pixels[i] < min) {
			min = pixels[i];
		}
		if (pixels[i] > max) {
			max = pixels[i];
		}
	}
	double sub = max - min;
	double ratio;
	for (i = 0; i < length; i++) {
		ratio = (double) (pixels[i] - min) / sub;
		pixels[i] = ratio * 255;
	}
}
#endif

#ifdef NORMALIZATION_PRAGMA
void min_max_normalization(uint32 *pixels, uint32 width, uint32 height) {
	uint32 min = INT_MAX, max = 0, i;
	uint32 length = height * width;
	for (i = 0; i < length; i++) {
		if (pixels[i] < min) {
			min = pixels[i];
		}
		if (pixels[i] > max) {
			max = pixels[i];
		}
	}
	double sub = max - min;
	double ratio;
	#pragma vector_for
	for (i = 0; i < length; i++) {
		ratio = (double) (pixels[i] - min) / sub;
		pixels[i] = ratio * 255;
	}
}
#endif


#ifdef KNOWN_IMAGE_SIZE_NORMALIZATION
/**
 * @brief Normalizing array to 0-255, used when image size if known at compile time
 *
 * @param pixels Array of pixels to be normalized
 * @param width Width of image
 * @param height Height of image
 */
void min_max_normalization(uint32 *pixels, uint32 width, uint32 height) {
	uint32 min = INT_MAX, max = 0, i;
	uint32 length = H * W;
	for (i = 0; i < length; i++) {
		if (pixels[i] < min) {
			min = pixels[i];
		}
		if (pixels[i] > max) {
			max = pixels[i];
		}
	}
	double sub = max - min;
	double ratio;
#pragma vector_for
	for (i = 0; i < length; i++) {
		ratio = (double) (pixels[i] - min) / sub;
		pixels[i] = ratio * 255;
	}
}
#endif
