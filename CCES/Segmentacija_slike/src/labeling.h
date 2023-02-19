/*
 * labeling.h
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#ifndef LABELING_H_
#define LABELING_H_

/**
 * @brief Labeling image using Conected Component algorithm
 *
 * @param edge_im Array of pixels
 * @param w Width of image
 * @param h Height of image
 */
void labeling(byte * edge_im, uint32 w, uint32 h);

#endif /* LABELING_H_ */
