/*
 * read_write.h
 *
 *  Created on: 19.02.2023.
 *      Author: nikol
 */

#ifndef READ_WRITE_H_
#define READ_WRITE_H_

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


/**
 * @brief specify image type, GRAY or Colored
 *
 */
typedef enum im_type {
	GRAY, COLORED,CODED
} type;

extern RGB colormap[SIZE];
/**
 * @brief Reading image from filesystem
 * @details This function is used when image size is unknown at compile time
 * @param fileName
 */
void ReadImage(const char *fileName);


/**
 * @brief Writing image to filesystem
 *
 * @param fileName Name of file
 * @param pixels Array of bytes
 * @param t Type of image, GRAY or COLORED
 */
void WriteImage(const char *fileName, byte* pixels, type t);

#endif /* READ_WRITE_H_ */
