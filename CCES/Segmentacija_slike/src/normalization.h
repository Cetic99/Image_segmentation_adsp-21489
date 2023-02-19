/*
 * normalization.h
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#ifndef NORMALIZATION_H_
#define NORMALIZATION_H_

/**
 * @brief Normalizing array to 0-255
 *
 * @param pixels Array of pixels to be normalized
 * @param width Width of image
 * @param height Height of image
 */
void min_max_normalization(uint32 *pixels, uint32 width, uint32 height);

#endif /* NORMALIZATION_H_ */
