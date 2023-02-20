/*
 * sobel_edge_detection.c
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#include "Segmentacija_slike.h"
#include "sobel_edge_detection.h"
#include "normalization.h"


#ifdef EDGE
/**
 * @brief Edge detection using Sobel kernel, no optimization
 *
 * @param pixels Input Array of pixels
 * @param out_pixels Output Array of pixels
 * @param width Width of image
 * @param height Height of image
 */
void sobel_edge_detector(byte * restrict pixels, byte ** out_pixels, uint32 width,
		uint32 height) {
	uint32 i, j, gx, gy;

	*out_pixels = pixels;         // assigning array to output pointer

	uint32 * restrict out_arr = (uint32 *) heap_malloc(0,
			width * height * sizeof(uint32));
	if (out_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	*out_pixels = out_arr;         // assigning array to output pointer
	uint32 (*edged_pixels_mat_uint32)[width] =
			(uint32 (*)[width]) out_arr; // converting array to matrix

	int32 mx[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
	int32 my[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };

	for (i = 1; i < height - 1; i++) {
		for (j = 1; j < width - 1; j++) {
			gx = convolution(pixels, mx, i, j, width);
			gy = convolution(pixels, my, i, j, width);
			edged_pixels_mat_uint32[i][j] = gx * gx + gy * gy;
		}
	}
	min_max_normalization(out_arr, width, height);

	heap_free(0,pixels);
}
#endif

#ifdef KNOWN_IMAGE_SIZE_EDGE_OPTIMIZED
/**
 * @brief Edge detection using Sobel kernel, used when image size is known at compile time
 *
 * @param pixels Input Array of pixels
 * @param out_pixels Output Array of pixels
 * @param width Width of image
 * @param height Height of image
 */
void sobel_edge_detector(byte * restrict pixels, byte **out_pixels, uint32 width,
		uint32 height) {
	uint32 i, j, gx, gy;


	uint32 *restrict edged_pixels_arr_uint32 = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (edged_pixels_arr_uint32 == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	*out_pixels = edged_pixels_arr_uint32;         // assigning array to output pointer

	uint32 (*edged_pixels_mat_uint32)[W] = (uint32 (*)[W]) edged_pixels_arr_uint32; // converting array to matrix

	int32 mx[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
	int32 my[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };

	for (i = 1; i < H - 1; i++) {
		for (j = 1; j < W - 1; j++) {
			gx = convolution(pixels, mx, i, j, width);
			gy = convolution(pixels, my, i, j, width);
			edged_pixels_mat_uint32[i][j] = gx * gx + gy * gy;//sqrt(gx * gx + gy * gy);
		}
	}
	min_max_normalization(edged_pixels_arr_uint32, W, H);

	heap_free(0,pixels);
}
#endif
