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

#define EDGE_VAL                0
#define NUM_LABELS              50000

//---------------------TYPES DEFINITION-------------------
typedef unsigned int uint32;
typedef signed int int32;
typedef short int16;
typedef unsigned char byte;
//========================================================

//-----------------MEMORY MAPPING SECTIONS----------------
extern const uint32 uid;
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


//#define READ_1 ||KNOWN_IMAGE_SIZE_READ
//#define WRITE_1
//#define GRAY_1
//#define CONVOLUTION_NO_OPT || CONVOLUTION_UNROLL || CONVOLUTION_PRAGMA || CONVOLUTION_UNROLL_INLINE || CONVOLUTION_PRAGMA_INLINE
//#define NORMALIZATION_NO_OPT || NORMALIZATION_HARDWARE || KNOWN_IMAGE_SIZE_NORMALIZATION
//#define EDGE || KNOWN_IMAGE_SIZE_EDGE_OPTIMIZED
//#define LABELING_V1 || LABELING_V2 || LABELING_V2_EXPECTED || KNOWN_IMAGE_SIZE_LABELING_OPTIMIZED

//-----------------------COMPILING------------------------
extern const char * filename;
//NO OPTIMIZATION
#define READ_1
#define WRITE_1
#define GRAY_1
#define CONVOLUTION_PRAGMA_INLINE
#define NORMALIZATION_HARDWARE
#define EDGE
#define LABELING_V2_EXPECTED
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

// //OPTIMIZED CONVOLUTION FUNCTION INLINE
// #define READ_1
// #define WRITE_1
// #define GRAY_1
// #define CONVOLUTION_UNROLL_INLINE
// #define NORMALIZATION_PRAGMA
// #define EDGE_NO_OPT
// #define LABELING_V2
// #define COLOR_IMAGE_NO_OPT


//KNOWN IMAGE SIZE
// #define KNOWN_IMAGE_SIZE_READ
// #define GRAY_PIPELINE
// #define CONVOLUTION_UNROLL_INLINE_PIPELINE
// #define KNOWN_IMAGE_SIZE_EDGE_OPTIMIZED
// #define KNOWN_IMAGE_SIZE_LABELING_OPTIMIZED
// #define KNOWN_IMAGE_SIZE_NORMALIZATION
// #define WRITE_1
// #define COLOR_IMAGE_PIPELINE




//========================================================

//--------USED ONLY WHEN KNOW_IMAGE_SIZE IS DEFINED-------
#define H 88
#define W 95
//========================================================

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
