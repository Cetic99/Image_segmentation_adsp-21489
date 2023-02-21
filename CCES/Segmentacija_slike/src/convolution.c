/*
 * convolution.c
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#include "Segmentacija_slike.h"
#include"convolution.h"

#ifdef CONVOLUTION_NO_OPT
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
uint32 convolution(byte * restrict pixels, int32 kernel[3][3], uint32 row, uint32 column, uint32 width)
{
	byte(*pix_mat)[width] = (byte(*)[width])pixels;
	uint32 i, j, sum = 0;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			sum += kernel[i][j] * pix_mat[i + row - 1][j + column - 1];
		}
	}
	return sum;
}
#endif

#ifdef CONVOLUTION_PRAGMA
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
uint32 convolution(byte *restrict pixels, int32 kernel[3][3], uint32 row, uint32 column, uint32 width)
{
	byte(*pix_mat)[width] = (byte(*)[width])pixels;
	uint32 i, j, sum = 0;
	#pragma vector_for
	for (i = 0; i < 3; i++)
	{
		sum += kernel[i][0] * pix_mat[i + row - 1][column - 1];
		sum += kernel[i][1] * pix_mat[i + row - 1][1 + column - 1];
		sum += kernel[i][2] * pix_mat[i + row - 1][2 + column - 1];
	}
	return sum;
}
#endif

#ifdef CONVOLUTION_UNROLL
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
uint32 convolution(byte * restrict pixels, int32 kernel[3][3], uint32 row,
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
