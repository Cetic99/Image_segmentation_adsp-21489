/**
 * @file Segmentacija_slike.c
 * @author Nikola Cetić (nikolacetic8@gmail.com)
 * @brief This file contains everything that is needed for image segmentation
 * @version 0.1
 * @date 2023-02-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
/*****************************************************************************
 * Segmentacija_slike.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "Segmentacija_slike.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <cycle_count.h>
#include <def21489.h> 
#include <sru21489.h> 
#include <SYSREG.h>


#define DATA_OFFSET_OFFSET      0x000A
#define WIDTH_OFFSET            0x0012
#define HEIGHT_OFFSET           0x0016
#define BITS_PER_PIXEL_OFFSET   0x001C
#define HEADER_SIZE             14
#define INFO_HEADER_SIZE        40
#define NO_COMPRESION           0
#define MAX_NUMBER_OF_COLORS    0
#define GRAY_COLOR              256
#define ALL_COLORS_REQUIRED     0
#define EDGE_VAL                0
#define NUM_LABELS              50000




//-----------------------COMPILING------------------------
const char * filename = "67x56.bmp";
//NO OPTIMIZATION
#define READ_1
#define WRITE_1
#define GRAY_1
#define CONVOLUTION_NO_OPT
#define NORMALIZATION_NO_OPT
#define EDGE_NO_OPT
#define LABELING_V1
#define COLOR_IMAGE_NO_OPT


// //OPTIMIZED LABELING FUNCTION
// #define READ_1
// #define WRITE_1
// #define GRAY_1
// #define CONVOLUTION_NO_OPT
// #define NORMALIZATION_PRAGMA
// #define EDGE_NO_OPT
// #define LABELING_V2
// #define COLOR_IMAGE_NO_OPT

// //OPTIMIZED CONVOLUTION FUNCTION
// #define READ_1
// #define WRITE_1
// #define GRAY_1
// #define CONVOLUTION_UNROLL
// #define NORMALIZATION_PRAGMA
// #define EDGE_NO_OPT
// #define LABELING_V2
// #define COLOR_IMAGE_NO_OPT


//KNOWN IMAGE SIZE
// #define KNOWN_IMAGE_SIZE_READ 				
// #define GRAY_1
// #define CONVOLUTION_UNROLL						
// #define KNOWN_IMAGE_SIZE_EDGE_OPTIMIZED		
// #define KNOWN_IMAGE_SIZE_LABELING_OPTIMIZED	
// #define KNOWN_IMAGE_SIZE_NORMALIZATION	
// #define WRITE_1
// #define COLOR_IMAGE_NO_OPT


//========================================================

//--------USED ONLY WHEN KNOW_IMAGE_SIZE IS DEFINED-------
#define H 88
#define W 95
//========================================================


//---------------------TYPES DEFINITION-------------------
typedef unsigned int uint32;
typedef signed int int32;
typedef short int16;
typedef unsigned char byte;
//========================================================


//-----------------MEMORY MAPPING SECTIONS----------------
uint32 uid = 999;
#pragma section("pixels_3b")
static byte pixels_3b[45001];
int index_pixels_3b;

//========================================================


//-------------------VARIABLE DECLARATION-----------------
byte *pixels;
byte *gray_pix_arr;
byte *edged_pix_array;
uint32 width;
uint32 height;
uint32 bytesPerPixel;
//========================================================


/**
 * @brief specify image type, GRAY or Colored
 * 
 */
typedef enum im_type {
	GRAY, COLORED
} type;

#define SIZE 256
/**
 * @brief Used for keeping colormap
 * 
 */
typedef struct rgb {
	byte r, g, b;
} RGB;
RGB colormap[SIZE] = { 0 };


/**
 * @brief Create a colormap object
 * 
 */
void create_colormap(void) {
	srand(time(0));
#pragma SIMD_for
	for (int i = 1; i < SIZE; i++) {
		colormap[i].r = rand();
		colormap[i].g = rand();
		colormap[i].b = rand();
	}
}

#ifdef KNOWN_IMAGE_SIZE_READ
/**
 * @brief Reading image from filesystem
 * @details This is used when image size is known at compile time
 * 
 * @param fileName 
 */
void ReadImage(const char *fileName) {
	FILE *imageFile = fopen(fileName, "r");
	if (imageFile == NULL) {
		printf("Nije otvorena slika\n");
		return;
	}
	char temp[4];
	uint32 dataOffset;
	fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	fread(temp + 2, 1, 1, imageFile);
	fread(temp + 3, 1, 1, imageFile);
	dataOffset = temp[3];
	dataOffset <<= 8;
	dataOffset |= temp[2];
	dataOffset <<= 8;
	dataOffset |= temp[1];
	dataOffset <<= 8;
	dataOffset |= temp[0];
	printf("DataOffset: %d\n", dataOffset);

	fseek(imageFile, WIDTH_OFFSET, SEEK_SET);

	//fread(&width, 4, 1, imageFile);
	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	fread(temp + 2, 1, 1, imageFile);
	fread(temp + 3, 1, 1, imageFile);
	width = temp[3];
	width <<= 8;
	width |= temp[2];
	width <<= 8;
	width |= temp[1];
	width <<= 8;
	width |= temp[0];

	printf("Width: %d\n", width);
	fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
	//fread(&height, 4, 1, imageFile);

	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	fread(temp + 2, 1, 1, imageFile);
	fread(temp + 3, 1, 1, imageFile);
	height = temp[3];
	height <<= 8;
	height |= temp[2];
	height <<= 8;
	height |= temp[1];
	height <<= 8;
	height |= temp[0];
	printf("Height: %d\n", height);
	int16 bitsPerPixel;
	fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
	//fread(&bitsPerPixel, 2, 1, imageFile);

	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	bitsPerPixel = temp[1];
	bitsPerPixel <<= 8;
	bitsPerPixel |= temp[0];
	printf("BitsPerPixel: %d\n", bitsPerPixel);
	bytesPerPixel = ((uint32) bitsPerPixel) / 8;
	printf("BytesPerPixel %d\n", bytesPerPixel);

	int paddedRowSize = (int) (4 * ceil((float) (width) / 4.0f))
			* (bytesPerPixel);
	int unpaddedRowSize = (width) * (bytesPerPixel);
	int totalSize = unpaddedRowSize * (height);

	index_pixels_3b = heap_install(pixels_3b, sizeof(pixels_3b), uid);
	if (index_pixels_3b < 0) {
		printf("Instalacija heap-a nije prosla\n");
		return;
	}
	pixels = (byte *) heap_malloc(index_pixels_3b, H * W);
	if (pixels == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}

	byte *currentRowPointer = pixels + ((H - 1) * unpaddedRowSize);
	for (int i = 0; i < H; i++) {
		fseek(imageFile, dataOffset + (i * paddedRowSize), SEEK_SET);
		fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
		currentRowPointer -= unpaddedRowSize;
	}

	fclose(imageFile);
	// printf("Slika ucitana!\n");
}
#endif

#ifdef READ_1
/**
 * @brief Reading image from filesystem
 * @details This function is used when image size is unknown at compile time
 * @param fileName 
 */
void ReadImage(const char *fileName) {
	FILE *imageFile = fopen(fileName, "r");
	if (imageFile == NULL) {
		printf("Nije otvorena slika\n");
		return;
	}
	char temp[4];
	uint32 dataOffset;
	fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	fread(temp + 2, 1, 1, imageFile);
	fread(temp + 3, 1, 1, imageFile);
	dataOffset = temp[3];
	dataOffset <<= 8;
	dataOffset |= temp[2];
	dataOffset <<= 8;
	dataOffset |= temp[1];
	dataOffset <<= 8;
	dataOffset |= temp[0];
	printf("DataOffset: %d\n", dataOffset);

	fseek(imageFile, WIDTH_OFFSET, SEEK_SET);

	//fread(&width, 4, 1, imageFile);
	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	fread(temp + 2, 1, 1, imageFile);
	fread(temp + 3, 1, 1, imageFile);
	width = temp[3];
	width <<= 8;
	width |= temp[2];
	width <<= 8;
	width |= temp[1];
	width <<= 8;
	width |= temp[0];

	printf("Width: %d\n", width);
	fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
	//fread(&height, 4, 1, imageFile);

	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	fread(temp + 2, 1, 1, imageFile);
	fread(temp + 3, 1, 1, imageFile);
	height = temp[3];
	height <<= 8;
	height |= temp[2];
	height <<= 8;
	height |= temp[1];
	height <<= 8;
	height |= temp[0];
	printf("Height: %d\n", height);
	int16 bitsPerPixel;
	fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
	//fread(&bitsPerPixel, 2, 1, imageFile);

	fread(temp, 1, 1, imageFile);
	fread(temp + 1, 1, 1, imageFile);
	bitsPerPixel = temp[1];
	bitsPerPixel <<= 8;
	bitsPerPixel |= temp[0];
	printf("BitsPerPixel: %d\n", bitsPerPixel);
	bytesPerPixel = ((uint32) bitsPerPixel) / 8;
	printf("BytesPerPixel %d\n", bytesPerPixel);

	int paddedRowSize = (int) (4 * ceil((float) (width) / 4.0f))
			* (bytesPerPixel);
	int unpaddedRowSize = (width) * (bytesPerPixel);
	int totalSize = unpaddedRowSize * (height);

	index_pixels_3b = heap_install(pixels_3b, sizeof(pixels_3b), uid);
	if (index_pixels_3b < 0) {
		printf("Instalacija heap-a nije prosla\n");
		return;
	}
	pixels = (byte *) heap_malloc(index_pixels_3b, height * width);
	if (pixels == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	int i = 0;
	byte *currentRowPointer = pixels + ((height - 1) * unpaddedRowSize);
	for (i = 0; i < height; i++) {
		fseek(imageFile, dataOffset + (i * paddedRowSize), SEEK_SET);
		fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
		currentRowPointer -= unpaddedRowSize;
	}

	fclose(imageFile);
	// printf("Slika ucitana!\n");
}
#endif

#ifdef WRITE_1
/**
 * @brief Writing image to filesystem
 * 
 * @param fileName Name of file
 * @param pixels Array of bytes
 * @param t Type of image, GRAY or COLORED
 */
void WriteImage(const char *fileName, byte* pixels, type t) {
	uint32 bytesPerPixel;
	if (t == GRAY)
		bytesPerPixel = 1;
	else
		bytesPerPixel = 3;
	FILE *outputFile = fopen(fileName, "w");
	if (!outputFile) {
		printf("fopen was not successful\n");
		return;
	}
	//*****HEADER************//
	const char *BM = "BM";
	fwrite(&BM[0], 1, 1, outputFile);
	fwrite(&BM[1], 1, 1, outputFile);
	int paddedRowSize = (int) (4 * ceil((float) width / 4.0f)) * bytesPerPixel;

	// Because ADSP-21489 is not byte addresable, it is needed to shift data and write it
	uint32 fileSize = paddedRowSize * height + HEADER_SIZE + INFO_HEADER_SIZE;
	printf("fileSize: %d\n", fileSize);
	fwrite(&fileSize, 1, 1, outputFile);
	fileSize >>= 8;
	fwrite(&fileSize, 1, 1, outputFile);
	fileSize >>= 8;
	fwrite(&fileSize, 1, 1, outputFile);
	fileSize >>= 8;
	fwrite(&fileSize, 1, 1, outputFile);

	uint32 reserved = 0x0000;
	fwrite(&reserved, 1, 1, outputFile);
	fwrite(&reserved, 1, 1, outputFile);
	fwrite(&reserved, 1, 1, outputFile);
	fwrite(&reserved, 1, 1, outputFile);

	uint32 dataOffset = HEADER_SIZE + INFO_HEADER_SIZE;
	uint32 dataOffset2 = dataOffset;
	if (t == GRAY) {
		dataOffset += 4 * 256;
	}
	fwrite(&dataOffset, 1, 1, outputFile);
	dataOffset >>= 8;
	fwrite(&dataOffset, 1, 1, outputFile);
	dataOffset >>= 8;
	fwrite(&dataOffset, 1, 1, outputFile);
	dataOffset >>= 8;
	fwrite(&dataOffset, 1, 1, outputFile);

	//*******INFO*HEADER******//
	uint32 infoHeaderSize = INFO_HEADER_SIZE;
	fwrite(&infoHeaderSize, 1, 1, outputFile);
	infoHeaderSize >>= 8;
	fwrite(&infoHeaderSize, 1, 1, outputFile);
	infoHeaderSize >>= 8;
	fwrite(&infoHeaderSize, 1, 1, outputFile);
	infoHeaderSize >>= 8;
	fwrite(&infoHeaderSize, 1, 1, outputFile);

	uint32 width2 = width;
	fwrite(&width2, 1, 1, outputFile);
	width2 >>= 8;
	fwrite(&width2, 1, 1, outputFile);
	width2 >>= 8;
	fwrite(&width2, 1, 1, outputFile);
	width2 >>= 8;
	fwrite(&width2, 1, 1, outputFile);

	uint32 height2 = height;
	fwrite(&height2, 1, 1, outputFile);
	height2 >>= 8;
	fwrite(&height2, 1, 1, outputFile);
	height2 >>= 8;
	fwrite(&height2, 1, 1, outputFile);
	height2 >>= 8;
	fwrite(&height2, 1, 1, outputFile);

	int16 planes = 1; // always 1
	fwrite(&planes, 1, 1, outputFile);
	planes >>= 8;
	fwrite(&planes, 1, 1, outputFile);

	int16 bitsPerPixel = bytesPerPixel * 8;
	fwrite(&bitsPerPixel, 1, 1, outputFile);
	bitsPerPixel >>= 8;
	fwrite(&bitsPerPixel, 1, 1, outputFile);

	// write compression
	uint32 compression = NO_COMPRESION;
	fwrite(&compression, 1, 1, outputFile);
	compression >>= 8;
	fwrite(&compression, 1, 1, outputFile);
	compression >>= 8;
	fwrite(&compression, 1, 1, outputFile);
	compression >>= 8;
	fwrite(&compression, 1, 1, outputFile);

	// write image size (in bytes)
	uint32 imageSize;
	uint32 resolutionX;
	uint32 resolutionY;
	if (t == GRAY) {
		imageSize = 0;
		resolutionX = 0; // 11811; //300 dpi
		resolutionY = 0; // 11811; //300 dpi
	} else {
		imageSize = width * height * bytesPerPixel;
		resolutionX = 0;
		resolutionY = 0;
	}
	fwrite(&imageSize, 1, 1, outputFile);
	imageSize >>= 8;
	fwrite(&imageSize, 1, 1, outputFile);
	imageSize >>= 8;
	fwrite(&imageSize, 1, 1, outputFile);
	imageSize >>= 8;
	fwrite(&imageSize, 1, 1, outputFile);

	fwrite(&resolutionX, 1, 1, outputFile);
	resolutionX >>= 8;
	fwrite(&resolutionX, 1, 1, outputFile);
	resolutionX >>= 8;
	fwrite(&resolutionX, 1, 1, outputFile);
	resolutionX >>= 8;
	fwrite(&resolutionX, 1, 1, outputFile);

	fwrite(&resolutionY, 1, 1, outputFile);
	resolutionY >>= 8;
	fwrite(&resolutionY, 1, 1, outputFile);
	resolutionY >>= 8;
	fwrite(&resolutionY, 1, 1, outputFile);
	resolutionY >>= 8;
	fwrite(&resolutionY, 1, 1, outputFile);

	uint32 colorsUsed = MAX_NUMBER_OF_COLORS;
	fwrite(&colorsUsed, 1, 1, outputFile);
	colorsUsed >>= 8;
	fwrite(&colorsUsed, 1, 1, outputFile);
	colorsUsed >>= 8;
	fwrite(&colorsUsed, 1, 1, outputFile);
	colorsUsed >>= 8;
	fwrite(&colorsUsed, 1, 1, outputFile);

	uint32 importantColors = ALL_COLORS_REQUIRED;
	fwrite(&importantColors, 1, 1, outputFile);
	importantColors >>= 8;
	fwrite(&importantColors, 1, 1, outputFile);
	importantColors >>= 8;
	fwrite(&importantColors, 1, 1, outputFile);
	importantColors >>= 8;
	fwrite(&importantColors, 1, 1, outputFile);

	int unpaddedRowSize = width * bytesPerPixel;
	if (t == GRAY) {
		int zero = 0;
		int counter = 0;
		for (unsigned char i = 0; i < 256; i++) {
			fwrite(&counter, 1, 1, outputFile);
			fwrite(&counter, 1, 1, outputFile);
			fwrite(&counter, 1, 1, outputFile);
			fwrite(&zero, 1, 1, outputFile);
			counter = counter + 1;
		}
	}

	int pixelOffset;
	for (int i = 0; i < height; i++) {
		// printf("Upis reda\n");
		pixelOffset = ((height - i) - 1) * unpaddedRowSize;
		fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
	}
	fclose(outputFile);
	// printf("Slika sacuvana!\n");
}
#endif

#ifdef GRAY_1
/**
 * @brief Conversion to Grayscale image
 * 
 */
void to_gray(void) {
	bytesPerPixel = 1;
	gray_pix_arr = (byte *) heap_malloc(0, height * width);
	if (gray_pix_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	int pix_position;
	int num_iter = height * width;
	for (int i = 0; i < num_iter; i++) {
		pix_position = i * 3;
		//                 ------RED--------                 --------GREEN--------               ------BLUE-------
		gray_pix_arr[i] = (char) ((float) pixels[pix_position + 2] * 0.299
				+ (float) pixels[pix_position + 1] * 0.587
				+ (float) pixels[pix_position] * 0.114);
	}
}
#endif

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
static inline uint32 convolution(byte *pixels, int32 kernel[3][3], uint32 row, uint32 column, uint32 width)
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
static inline uint32 convolution(byte *pixels, uint32 kernel[3][3], uint32 row, uint32 column, uint32 width)
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
static inline uint32 convolution(byte *pixels, int32 kernel[3][3], uint32 row,
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


#ifdef EDGE_NO_OPT
/**
 * @brief Edge detection using Sobel kernel, no optimization
 * 
 * @param pixels Input Array of pixels
 * @param out_pixels Output Array of pixels
 * @param width Width of image
 * @param height Height of image
 */
void sobel_edge_detector(byte *pixels, byte **out_pixels, uint32 width,
		uint32 height) {
	uint32 i, j, gx, gy;

	//byte *edged_pixels_arr = (byte *)malloc(width * height);                            // creating array of bytes
	*out_pixels = pixels;         // assigning array to output pointer

	uint32 *edged_pixels_arr_uint32 = (uint32 *) heap_malloc(0,
			width * height * sizeof(uint32));
	if (edged_pixels_arr_uint32 == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	//uint32 *edged_pixels_arr_uint32 = (uint32 *)malloc(width * height * sizeof(uint32));    // creating array of uint32
	uint32 (*edged_pixels_mat_uint32)[width] =
			(uint32 (*)[width]) edged_pixels_arr_uint32; // converting array to matrix

	int32 mx[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
	int32 my[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };

	for (i = 1; i < height - 1; i++) {
		for (j = 1; j < width - 1; j++) {
			gx = convolution(pixels, mx, i, j, width);
			gy = convolution(pixels, my, i, j, width);
			edged_pixels_mat_uint32[i][j] = sqrt(gx * gx + gy * gy);
		}
		// printf("Convolved!\n");
	}
	// printf("Done with convolution!\n");
	min_max_normalization(edged_pixels_arr_uint32, width, height);
	for (i = 0; i < height * width; i++) {
		pixels[i] = edged_pixels_arr_uint32[i]; //converting from 32bit to 8bit
		// printf("%d",edged_pixels_arr[i]);
	}
	heap_free(0,edged_pixels_arr_uint32);
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
void sobel_edge_detector(byte *pixels, byte **out_pixels, uint32 width,
		uint32 height) {
	uint32 i, j, gx, gy;
	*out_pixels = pixels;         // assigning array to output pointer

	uint32 *edged_pixels_arr_uint32 = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (edged_pixels_arr_uint32 == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	uint32 (*edged_pixels_mat_uint32)[W] = (uint32 (*)[W]) edged_pixels_arr_uint32; // converting array to matrix

	int32 mx[3][3] = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
	int32 my[3][3] = { { -1, -2, -1 }, { 0, 0, 0 }, { 1, 2, 1 } };

	for (i = 1; i < H - 1; i++) {
		for (j = 1; j < W - 1; j++) {
			gx = convolution(pixels, mx, i, j, width);
			gy = convolution(pixels, my, i, j, width);
			edged_pixels_mat_uint32[i][j] = gx * gx + gy * gy;//sqrt(gx * gx + gy * gy);
		}
		// printf("Convolved!\n");
	}
	// printf("Done with convolution!\n");
	min_max_normalization(edged_pixels_arr_uint32, W, H);
	for (i = 0; i < H * W; i++) {
		pixels[i] = edged_pixels_arr_uint32[i]; //converting from 32bit to 8bit
		// printf("%d",edged_pixels_arr[i]);
	}
	heap_free(0,edged_pixels_arr_uint32);
}
#endif

#ifdef LABELING_V1
/**
 * @brief Labeling image using Conected Component algorithm
 * @details First type
 * 
 * @param edge_im Array of pixels
 * @param w Width of image
 * @param h Height of image
 */
void labeling(byte * edge_im, uint32 w,uint32 h)

{
	// binary conversion
	for(int i =0; i<w*h;i++)
	{
		if(edge_im[i] > 7)
		edge_im[i] = 0;
		else
		edge_im[i] = 1;
	}
	/*
	 Maybe is better to make new matrix with values of edges UINT_MAX
	 With this it will be possible to remove EDGE_VAL in if statement
	 WAITING FOR NEXT ITERATION!!!!!!!
	 */
	byte (*edge_mat)[w] = (byte(*)[w])edge_im;
	uint32 *parent = (uint32 *) heap_malloc(0, w * h * sizeof(uint32));
	if (parent == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	for(int i = 0;i<w*h;i++)                // Everyone is it's own parent
	{
		parent[i] = i;
	}
	uint32 label_counter = 1;
	uint32 min_neighbour;
	uint32 *mat_arr = (uint32 *) heap_malloc(0, w * h * sizeof(uint32));
	if (mat_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	for( int i = 0; i< w*h; i++) {
		mat_arr[i] = edge_im[i];
	}
	uint32 (*mat_val)[w] = (uint32(*)[w])mat_arr;

	for(int i = 1; i< h-1; i++)
	{
		for(int j = 1; j < w-1; j++)
		{
			min_neighbour = UINT_MAX;
			if(edge_mat[i][j] == EDGE_VAL)
			{
				continue;
			}
			if (mat_val[i-1][j-1] > EDGE_VAL && mat_val[i-1][j-1] < min_neighbour)
			{
				min_neighbour = mat_val[i-1][j-1];
			}
			if (mat_val[i-1][j] > EDGE_VAL && mat_val[i-1][j] < min_neighbour)
			{
				min_neighbour = mat_val[i-1][j];
			}
			if (mat_val[i-1][j+1] > EDGE_VAL && mat_val[i-1][j+1] < min_neighbour)
			{
				min_neighbour = mat_val[i-1][j+1];
			}
			if (mat_val[i][j-1] > EDGE_VAL && mat_val[i][j-1] < min_neighbour)
			{
				min_neighbour = mat_val[i][j-1];
			}
			if(min_neighbour == UINT_MAX)
			{
				label_counter++;
				min_neighbour = label_counter;
			}
			mat_val[i][j] = min_neighbour; //assigning this pixel its label value
			if (mat_val[i-1][j+1] > EDGE_VAL && mat_val[i-1][j+1] > min_neighbour)
			{
				parent[mat_val[i-1][j+1]] = min_neighbour;
			}
			if (mat_val[i-1][j] > EDGE_VAL && mat_val[i-1][j] > min_neighbour)
			{
				parent[mat_val[i-1][j]] = min_neighbour;
			}
		}
	}
	int i = 0;
	int j = 0;
	for(i = 0; i < label_counter; i++)
	{
		j = i;
		while(parent[j] != j) {
			j = parent[j];
		}
		parent[i] = j;
	}
	for(i = 0; i< h; i++)
	{
		for(j = 0; j < w; j++)
		{
			mat_val[i][j] = parent[mat_val[i][j]];
		}
	}
	min_max_normalization(mat_arr, w, h);
	for(i = 0; i<w*h; i++) {
		edge_im[i] = mat_arr[i];
	}
	heap_free(0,parent);
	heap_free(0,mat_arr);

}
#endif

#ifdef LABELING_V2
/**
 * @brief Labeling image using Conected Component algorithm
 * @details Second type
 * 
 * @param edge_im Array of pixels
 * @param w Width of image
 * @param h Height of image
 */
void labeling(byte * edge_im, uint32 w, uint32 h)

{
	// binary conversion
#pragma SIMD_for
	for (int i = 0; i < w * h; i++) {
		if (edge_im[i] > 7)
			edge_im[i] = 0;
		else
			edge_im[i] = 1;
	}

	byte (*edge_mat)[w] = (byte (*)[w]) edge_im;
	uint32 *parent_arr = (uint32 *) heap_malloc(0, w * h * sizeof(uint32));
	if (parent_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
#pragma vector_for
	for (int i = 0; i < w*h; i++)          // Everyone is it's own parent
	{
		parent_arr[i] = i;
	}
	uint32 label_counter = w*h;
	uint32 max_neighbour;
	uint32 *mat_arr = (uint32 *) heap_malloc(0, w * h * sizeof(uint32));
	if (mat_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
#pragma vector_for(4)
	for (int i = 0; i < w * h; i++) {
		mat_arr[i] = edge_im[i];
	}
	uint32 (*mat_val)[w] = (uint32 (*)[w]) mat_arr;

	for (int i = 1; i < h - 1; i++) {
		for (int j = 1; j < w - 1; j++) {
			max_neighbour = 1;
			if (edge_mat[i][j] == EDGE_VAL) {
				continue;
			}
			if (mat_val[i - 1][j - 1] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j - 1];
			}
			if (mat_val[i - 1][j] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j];
			}
			if (mat_val[i - 1][j + 1] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j + 1];
			}
			if (mat_val[i][j - 1] > max_neighbour) {
				max_neighbour = mat_val[i][j - 1];
			}
			if (max_neighbour == 1) {
				label_counter--;
				max_neighbour = label_counter;
			}
			mat_val[i][j] = max_neighbour; //assigning this pixel its label value
			if (mat_val[i - 1][j + 1] < max_neighbour) {
				parent_arr[mat_val[i - 1][j + 1]] = max_neighbour;
			}
			if (mat_val[i - 1][j] < max_neighbour) {
				parent_arr[mat_val[i - 1][j]] = max_neighbour;
			}
		}
	}
	int i = 0;
	int j = 0;
#pragma SIMD_for
	for (i = w*h; i >= label_counter; i--) {
		j = i;
		while (parent_arr[j] != j) {
			j = parent_arr[j];
		}
		parent_arr[i] = j;
	}
#pragma vector_for
	for (i = 0; i < h * w; i++) {
		mat_arr[i] = parent_arr[mat_arr[i]];
	}
	min_max_normalization(mat_arr, w, h);
#pragma SIMD_for
	for (i = 0; i < w * h; i++) {
		edge_im[i] = mat_arr[i];
	}
	heap_free(0,parent_arr);
	heap_free(0,mat_arr);
}
#endif

#ifdef KNOWN_IMAGE_SIZE_LABELING_1
/**
 * @brief Labeling image using Conected Component algorithm
 * @details Used when image size is known at compile time, Without optimization
 * 
 * @param edge_im Array of pixels
 * @param w Width of image
 * @param h Height of image
 */
void labeling(byte * edge_im, uint32 w, uint32 h)

{
	// binary conversion
	for (int i = 0; i < W * H; i++) {
		if (edge_im[i] > 10)
			edge_im[i] = 0;
		else
			edge_im[i] = 1;
	}

	byte (*edge_mat)[W] = (byte (*)[W]) edge_im;
	uint32 *parent_arr = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (parent_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	//uint32 *parent = malloc(w*h*4);
	for (int i = 0; i < W*H; i++)          // Everyone is it's own parent
	{
		parent_arr[i] = i;
	}
	uint32 label_counter = W*H;
	uint32 max_neighbour;
	uint32 *mat_arr = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (mat_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	for (int i = 0; i < W * H; i++) {
		mat_arr[i] = edge_im[i];
	}
	uint32 (*mat_val)[W] = (uint32 (*)[W]) mat_arr;

	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) {
			max_neighbour = 1;
			if (edge_mat[i][j] == EDGE_VAL) {
				continue;
			}
			if (mat_val[i - 1][j - 1] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j - 1];
			}
			if (mat_val[i - 1][j] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j];
			}
			if (mat_val[i - 1][j + 1] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j + 1];
			}
			if (mat_val[i][j - 1] > max_neighbour) {
				max_neighbour = mat_val[i][j - 1];
			}
			if (max_neighbour == 1) {
				label_counter--;
				max_neighbour = label_counter;
			}
			mat_val[i][j] = max_neighbour; //assigning this pixel its label value
			if (mat_val[i - 1][j + 1] < max_neighbour) {
				parent_arr[mat_val[i - 1][j + 1]] = max_neighbour;
			}
			if (mat_val[i - 1][j] < max_neighbour) {
				parent_arr[mat_val[i - 1][j]] = max_neighbour;
			}
		}
	}
	int i = 0;
	int j = 0;
	for (i = W*H; i >= label_counter; i--) {
		j = i;
		while (parent_arr[j] != j) {
			j = parent_arr[j];
		}
		parent_arr[i] = j;
	}
	for (i = 0; i < H * W; i++) {
		mat_arr[i] = parent_arr[mat_arr[i]];
	}
	min_max_normalization(mat_arr, W, H);
	for (i = 0; i < W * H; i++) {
		edge_im[i] = mat_arr[i];
	}
	free(parent_arr);

}
#endif

#ifdef KNOWN_IMAGE_SIZE_LABELING_OPTIMIZED
/**
 * @brief Labeling image using Conected Component algorithm
 * @details Used when image size is known at compile time, With optimization
 * 
 * @param edge_im Array of pixels
 * @param w Width of image
 * @param h Height of image
 */
void labeling(byte * edge_im, uint32 w, uint32 h)

{
	// binary conversion
#pragma SIMD_for
	for (int i = 0; i < W * H; i++) {
		if (edge_im[i] > 10)
			edge_im[i] = 0;
		else
			edge_im[i] = 1;
	}

	byte (*edge_mat)[W] = (byte (*)[W]) edge_im;
	uint32 *parent_arr = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (parent_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
#pragma vector_for
	for (int i = 0; i < W*H; i++)          // Everyone is it's own parent
	{
		parent_arr[i] = i;
	}
	uint32 label_counter = W*H;
	uint32 max_neighbour;
	uint32 *mat_arr = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (mat_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
#pragma vector_for(4)
	for (int i = 0; i < W * H; i++) {
		mat_arr[i] = edge_im[i];
	}
	uint32 (*mat_val)[W] = (uint32 (*)[W]) mat_arr;


	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) {
			max_neighbour = 1;
			if (edge_mat[i][j] == EDGE_VAL) {
				continue;
			}
			if (mat_val[i - 1][j - 1] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j - 1];
			}
			if (mat_val[i - 1][j] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j];
			}
			if (mat_val[i - 1][j + 1] > max_neighbour) {
				max_neighbour = mat_val[i - 1][j + 1];
			}
			if (mat_val[i][j - 1] > max_neighbour) {
				max_neighbour = mat_val[i][j - 1];
			}
			if (max_neighbour == 1) {
				label_counter--;
				max_neighbour = label_counter;
			}
			mat_val[i][j] = max_neighbour; //assigning this pixel its label value
			if (mat_val[i - 1][j + 1] < max_neighbour) {
				parent_arr[mat_val[i - 1][j + 1]] = max_neighbour;
			}
			if (mat_val[i - 1][j] < max_neighbour) {
				parent_arr[mat_val[i - 1][j]] = max_neighbour;
			}
		}
	}
	int i = 0;
	int j = 0;
#pragma SIMD_for
	for (i = W*H; i >= label_counter; i--) {
		j = i;
		while (parent_arr[j] != j) {
			j = parent_arr[j];
		}
		parent_arr[i] = j;
	}
#pragma vector_for
	for (i = 0; i < H * W; i++) {
		mat_arr[i] = parent_arr[mat_arr[i]];
	}
	min_max_normalization(mat_arr, W, H);
#pragma SIMD_for
	for (i = 0; i < W * H; i++) {
		edge_im[i] = mat_arr[i];
	}
	heap_free(0,parent_arr);
	heap_free(0,mat_arr);
}
#endif

#ifdef COLOR_IMAGE_NO_OPT
/**
 * @brief Coloring image based on colormap created with function create_colormap()
 * 
 * @param labeled_im Input Array of pixels after labeling
 * @param colored_im Output Array of colored pixels
 * @param w Width of image
 * @param h Height of image
 */
void colorImage(byte * labeled_im, byte* colored_im, uint32 w, uint32 h) {
	uint32 num_iter = w * h;
	uint32 counter = 1;

	for (int i = 0; i < num_iter; i++) {
		counter = i * 3;
		//*((int*) (colored_im + counter)) = colormap[labeled_im[i]].number;
		colored_im[counter] = colormap[labeled_im[i]].r;
		colored_im[counter+1] = colormap[labeled_im[i]].g;
		colored_im[counter+2] = colormap[labeled_im[i]].b;
	}
}
#endif

/**
 * @brief Initialization of LEDs
 * 
 */
void InitSRU(void){	//** LED01**//
		SRU(HIGH,DPI_PBEN06_I);	
		SRU(FLAG4_O,DPI_PB06_I);	//** LED02**//	
		SRU(HIGH,DPI_PBEN13_I);	
		SRU(FLAG5_O,DPI_PB13_I);	//** LED03**//	
		SRU(HIGH,DPI_PBEN14_I);	
		SRU(FLAG6_O,DPI_PB14_I);	//** LED04**//	
		SRU(HIGH,DAI_PBEN03_I);
		SRU(HIGH,DAI_PB03_I);	//** LED05**//	
		SRU(HIGH,DAI_PBEN04_I);	
		SRU(HIGH,DAI_PB04_I);	//** LED06**//	
		SRU(HIGH,DAI_PBEN15_I);	
		SRU(HIGH,DAI_PB15_I);	//** LED07**//	
		SRU(HIGH,DAI_PBEN16_I);	
		SRU(HIGH,DAI_PB16_I);	//** LED08**//	
		SRU(HIGH,DAI_PBEN17_I);	
		SRU(HIGH,DAI_PB17_I);	//Setting flag pins as outputs
		sysreg_bit_set(sysreg_FLAGS, (FLG4O|FLG5O|FLG6O) );	//Setting HIGH to flag pins	
		sysreg_bit_set(sysreg_FLAGS, (FLG4|FLG5|FLG6) );
	}
/**
 * If you want to use command program arguments, then place them in the following string.
 */
char __argv_string[] = "";

cycle_t start_count;
cycle_t final_count;
cycle_t program_start;

int main() {
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	InitSRU();	//turn off LEDs	
	sysreg_bit_clr(sysreg_FLAGS, FLG4);	
	sysreg_bit_clr(sysreg_FLAGS, FLG5);	
	sysreg_bit_clr(sysreg_FLAGS, FLG6);	
	SRU(LOW,DAI_PB03_I);	
	SRU(LOW,DAI_PB04_I);	
	SRU(LOW,DAI_PB15_I);	
	SRU(LOW,DAI_PB16_I);	
	SRU(LOW,DAI_PB17_I);


	// reading Image into pixels
	sysreg_bit_set(sysreg_FLAGS, FLG4);	
	START_CYCLE_COUNT(program_start);
	START_CYCLE_COUNT(start_count);
	ReadImage(filename);
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za citanje: ",final_count);

	//WriteImage("lb", pixels, COLORED);

	//converting to gray
	
	sysreg_bit_set(sysreg_FLAGS, FLG5);	
	START_CYCLE_COUNT(start_count);
	to_gray();
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za grayscale: ",final_count);

	WriteImage("Gray", gray_pix_arr, GRAY);

	// detecting edges
	sysreg_bit_set(sysreg_FLAGS, FLG6);	
		
	START_CYCLE_COUNT(start_count);
	sobel_edge_detector(gray_pix_arr, &edged_pix_array, width, height);
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za detekciju ivica: ",final_count);

	WriteImage("Edged", edged_pix_array, GRAY);
	//labeling----coding image
	SRU(HIGH,DAI_PB03_I);	
	
	START_CYCLE_COUNT(start_count);
	labeling(edged_pix_array, width, height);
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za kodovanje slike: ",final_count);

	
	SRU(HIGH,DAI_PB04_I);	
	WriteImage("Coded", edged_pix_array, GRAY);
	START_CYCLE_COUNT(start_count);
	create_colormap();
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za kreiranje palete boja: ",final_count);

	SRU(HIGH,DAI_PB15_I);
	START_CYCLE_COUNT(start_count);
	colorImage(edged_pix_array, pixels, width, height);
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za bojenje slike: ",final_count);


	//writing image to file
	
	SRU(HIGH,DAI_PB16_I);
	START_CYCLE_COUNT(start_count);
	WriteImage("Out", pixels, COLORED);
	STOP_CYCLE_COUNT(final_count,start_count);
	PRINT_CYCLES("Broj ciklusa za cuvanje slike:  ",final_count);

	STOP_CYCLE_COUNT(final_count,program_start);
	PRINT_CYCLES("Broj ciklusa citav program: ",final_count);
	// freeing memory
	heap_free(0,pixels);
	SRU(HIGH,DAI_PB17_I);
	return 0;
}

