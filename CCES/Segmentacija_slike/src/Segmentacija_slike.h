/*****************************************************************************
 * Segmentacija_slike.h
 *****************************************************************************/

#ifndef __SEGMENTACIJA_SLIKE_H__
#define __SEGMENTACIJA_SLIKE_H__

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
#include <21489.h>



/*---------------------TYPES DEFINITION-------------------*/
typedef unsigned int uint32;
typedef signed int int32;
typedef int int16;
typedef unsigned int byte;
/*========================================================*/

/*-----------------MEMORY MAPPING SECTIONS----------------*/
extern const uint32 uid;
#pragma section("pixels_3b")
extern byte pixels_3b[45001];
extern int index_pixels_3b;

/*========================================================*/


/*-------------------VARIABLE DECLARATION-----------------*/
extern byte *pixels;
extern byte *gray_pix_arr;
extern byte *edged_pix_array;
extern uint32 width;
extern uint32 height;
extern uint32 bytesPerPixel;
/*========================================================*/

/*========================================================*/

/*--------USED ONLY WHEN KNOW_IMAGE_SIZE IS DEFINED-------*/
#define H 100
#define W 100
/*========================================================*/

/*-----------------------COMPILING------------------------*/
extern const char * filename;

/*
#define READ_1 ||KNOWN_IMAGE_SIZE_READ
#define WRITE_1
#define GRAY_1
#define CONVOLUTION_NO_OPT || CONVOLUTION_UNROLL || CONVOLUTION_PRAGMA || CONVOLUTION_PRAGMA_INLINE
#define NORMALIZATION_NO_OPT || NORMALIZATION_HARDWARE || KNOWN_IMAGE_SIZE_NORMALIZATION
#define EDGE || KNOWN_IMAGE_SIZE_EDGE_OPTIMIZED
#define LABELING_V1 || LABELING_V2 || LABELING_V2_EXPECTED || KNOWN_IMAGE_SIZE_LABELING_OPTIMIZED
*/



/*NO OPTIMIZATION*/
#define READ_1
#define WRITE_1
#define GRAY_1
#define CONVOLUTION_NO_OPT
#define NORMALIZATION_NO_OPT
#define EDGE
#define LABELING_V1
#define COLOR_IMAGE_NO_OPT




#define SIZE 256
/**
 * @brief Used for keeping colormap
 *
 */
typedef struct rgb {
	byte r, g, b;
} RGB;

/**
 * @brief Create a colormap object
 *
 */
void create_colormap(void);

/**
 * @brief Conversion to Grayscale image
 *
 */
void to_gray(byte * restrict pixels);

#endif /* __SEGMENTACIJA_SLIKE_H__ */
