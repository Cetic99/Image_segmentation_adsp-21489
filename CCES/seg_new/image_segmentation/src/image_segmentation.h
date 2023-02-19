/*****************************************************************************
 * image_segmentation.h
 *****************************************************************************/

#ifndef __IMAGE_SEGMENTATION_H__
#define __IMAGE_SEGMENTATION_H__

/* Add your custom header content here */

/* Add your custom header content here */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <cycle_count.h>
#include <def21489.h>
#include <sru21489.h>
#include <SYSREG.h>

//---------------------TYPES DEFINITION-------------------
typedef unsigned int uint32;
typedef signed int int32;
typedef short int16;
typedef unsigned char byte;
//========================================================

//-----------------MEMORY MAPPING SECTIONS----------------
const uint32 uid = 999;
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


//-----------------------COMPILING------------------------
const char * filename = "100x100.bmp";
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
#define H 100
#define W 100
//========================================================

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
#endif /* __IMAGE_SEGMENTATION_H__ */
