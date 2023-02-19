/*
 * read_write.c
 *
 *  Created on: 19.02.2023.
 *      Author: nikol
 */
#include"Segmentacija_slike.h"
#include "read_write.h"

#ifdef READ_1
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
	if (t == GRAY || t== CODED)
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
	if (t == GRAY || t == CODED) {
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
	if (t == GRAY || t== CODED) {
		imageSize = 0;
		resolutionX = 0; // 11811; //300 dpi
		resolutionY = 0; // 11811; //300 dpi
	} else if(COLORED) {
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
			if(counter != 10){
				fwrite(&counter, 1, 1, outputFile);
				fwrite(&counter, 1, 1, outputFile);
				fwrite(&counter, 1, 1, outputFile);
			}
			else{
				int random = 9;
				fwrite(&random, 1, 1, outputFile);
				fwrite(&random, 1, 1, outputFile);
				fwrite(&random, 1, 1, outputFile);
			}
			fwrite(&zero, 1, 1, outputFile);
			counter = counter + 1;
		}
	}
	else if (t == CODED) {
		int zero = 0;
		for (unsigned char i = 0; i < 256; i++) {
			fwrite(&colormap[i].b, 1, 1, outputFile);
			fwrite(&colormap[i].g, 1, 1, outputFile);
			fwrite(&colormap[i].r, 1, 1, outputFile);
			fwrite(&zero, 1, 1, outputFile);
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
