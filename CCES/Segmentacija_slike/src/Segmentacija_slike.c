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
#include "read_write.h"
#include "convolution.h"
#include "sobel_edge_detection.h"
#include "labeling.h"



const char * filename = "95x88.bmp";
const uint32 uid = 999;
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


#ifdef GRAY_1
/**
 * @brief Conversion to Grayscale image
 * 
 */
static inline void to_gray(byte * restrict pixels) {
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

#ifdef COLOR_IMAGE_PIPELINE
/**
 * @brief Coloring image based on colormap created with function create_colormap()
 *
 * @param labeled_im Input Array of pixels after labeling
 * @param colored_im Output Array of colored pixels
 * @param w Width of image
 * @param h Height of image
 */
void colorImage(byte * restrict labeled_im, byte* restrict colored_im, uint32 w, uint32 h) {
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
	to_gray(pixels);
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

	WriteImage("Out_NEW", edged_pix_array, CODED);

	
	//OVERKILL...
//	SRU(HIGH,DAI_PB15_I);
//	START_CYCLE_COUNT(start_count);
//	colorImage(edged_pix_array, pixels, width, height);
//	STOP_CYCLE_COUNT(final_count,start_count);
//	PRINT_CYCLES("Broj ciklusa za bojenje slike: ",final_count);
//
//
//	//writing image to file
//
//	SRU(HIGH,DAI_PB16_I);
//	START_CYCLE_COUNT(start_count);
//	WriteImage("Out", pixels, COLORED);
//	STOP_CYCLE_COUNT(final_count,start_count);
//	PRINT_CYCLES("Broj ciklusa za cuvanje slike:  ",final_count);

	STOP_CYCLE_COUNT(final_count,program_start);
	PRINT_CYCLES("Broj ciklusa citav program: ",final_count);
	// freeing memory
	heap_free(0,pixels);
	SRU(HIGH,DAI_PB17_I);
	return 0;
}

