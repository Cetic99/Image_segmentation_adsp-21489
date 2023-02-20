/*
 * convolution.h
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#ifndef CONVOLUTION_H_
#define CONVOLUTION_H_


#if defined CONVOLUTION_NO_OPT || defined CONVOLUTION_PRAGMA || defined CONVOLUTION_UNROLL
/**
 * @brief 2D convolution of image, Without optimization
 *
 * @param pixels Array of pixels to be convolved
 * @param kernel 3x3 kernel for convolution
 * @param row Specifying row of main pixel
 * @param column Specifying column of main pixel
 * @param width Width of Image
 * @return uint32
 */
uint32 convolution(byte *pixels, int32 kernel[3][3], uint32 row, uint32 column, uint32 width);
#endif

#ifdef CONVOLUTION_UNROLL_INLINE
/**
 * @brief 2D convolution of image, unrolled loops
 *
 * @param pixels Array of pixels to be convolved
 * @param kernel 3x3 kernel for convolution
 * @param row Specifying row of main pixel
 * @param column Specifying column of main pixel
 * @param width Width of Image
 * @return uint32
 */
static inline uint32 convolution(byte * restrict pixels, int32 kernel[3][3], uint32 row,
		uint32 column, uint32 width) {
	byte (*pix_mat)[width] = (byte (*)[width]) pixels;
	uint32 i, j, sum = 0;
	sum += kernel[0][0] * pix_mat[row - 1][column - 1];
	sum += kernel[0][1] * pix_mat[row - 1][1 + column - 1];
	sum += kernel[0][2] * pix_mat[row - 1][2 + column - 1];

	sum += kernel[1][0] * pix_mat[1 + row - 1][column - 1];
	sum += kernel[1][1] * pix_mat[1 + row - 1][1 + column - 1];
	sum += kernel[1][2] * pix_mat[1 + row - 1][2 + column - 1];

	sum += kernel[2][0] * pix_mat[2 + row - 1][column - 1];
	sum += kernel[2][1] * pix_mat[2 + row - 1][1 + column - 1];
	sum += kernel[2][2] * pix_mat[2 + row - 1][2 + column - 1];
	return sum;
}
#endif

#ifdef CONVOLUTION_PRAGMA_INLINE
/**
 * @brief 2D convolution of image, Pragma Optimization,a and inside loop is unrolled
 *
 * @param pixels Array of pixels to be convolved
 * @param kernel 3x3 kernel for convolution
 * @param row Specifying row of main pixel
 * @param column Specifying column of main pixel
 * @param width Width of Image
 * @return uint32
 */
static inline uint32 convolution(byte * restrict pixels, int32 kernel[3][3], uint32 row, uint32 column, uint32 width)
{
	byte(*pix_mat)[width] = (byte(*)[width])pixels;
	int i, j;
	int sum = 0;
	for (i = 0; i < 3; i++)
	{
		sum += kernel[i][0] * pix_mat[i + row - 1][column - 1];
		sum += kernel[i][1] * pix_mat[i + row - 1][1 + column - 1];
		sum += kernel[i][2] * pix_mat[i + row - 1][2 + column - 1];
	}
	return sum;
}
#endif
#endif /* CONVOLUTION_H_ */
