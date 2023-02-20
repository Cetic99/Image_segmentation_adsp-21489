/*
 * labeling.c
 *
 *  Created on: 19.02.2020.
 *      Author: nikol
 */

#include "Segmentacija_slike.h"
#include "labeling.h"

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
	byte (*mat_val)[w] = (byte(*)[w])edge_im;
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
	for(int i = 1; i< h - 1 ; i++)
	{
		for(int j = 1; j < w - 1; j++)
		{
			min_neighbour = UINT_MAX;
			if(mat_val[i][j] == EDGE_VAL)
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
	min_max_normalization(edge_im, w, h);

	heap_free(0,parent);

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
void labeling(byte * restrict edge_im, uint32 w, uint32 h)

{
	// binary conversion
//#pragma SIMD_for
	for (int i = 0; i < w * h; i++) {
		if (edge_im[i] > 7)
			edge_im[i] = 0;
		else
			edge_im[i] = 1;
	}

	byte (*mat_val)[w] = (byte (*)[w]) edge_im;
	uint32 * restrict parent_arr = (uint32 *) heap_malloc(0, w * h * sizeof(uint32));
	if (parent_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
#pragma vector_for(4)
	for (int i = 0; i < w*h; i++)          // Everyone is it's own parent
	{
		parent_arr[i] = i;
	}
	uint32 label_counter = w*h;
	uint32 max_neighbour;


	for (int i = 1; i < h - 1; i++) {
		for (int j = 1; j < w - 1; j++) {
			max_neighbour = 1;
			if (mat_val[i][j] == EDGE_VAL) {
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
//#pragma SIMD_for
	for (i = w*h; i >= label_counter; i--) {
		j = i;
		while (parent_arr[j] != j) {
			j = parent_arr[j];
		}
		parent_arr[i] = j;
	}
	for (i = 0; i < h * w; i++) {
		edge_im[i] = parent_arr[edge_im[i]];
	}
	min_max_normalization(edge_im, w, h);

	heap_free(0,parent_arr);
}
#endif

#ifdef LABELING_V2_EXPECTED
/**
 * @brief Labeling image using Conected Component algorithm
 * @details Second type
 *
 * @param edge_im Array of pixels
 * @param w Width of image
 * @param h Height of image
 */
void labeling(byte * restrict edge_im, uint32 w, uint32 h)

{
	// binary conversion
//#pragma SIMD_for
	for (int i = 0; i < w * h; i++) {
		if (edge_im[i] > 7)
			edge_im[i] = 0;
		else
			edge_im[i] = 1;
	}

	byte (*mat_val)[w] = (byte (*)[w]) edge_im;
	uint32 * restrict parent_arr = (uint32 *) heap_malloc(0, w * h * sizeof(uint32));
	if (parent_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}
	for (int i = 0; i < w*h; i++)          // Everyone is it's own parent
	{
		parent_arr[i] = i;
	}
	uint32 label_counter = w*h;
	uint32 max_neighbour;


	for (int i = 1; i < h - 1; i++) {
		for (int j = 1; j < w - 1; j++) {
			max_neighbour = 1;
			if (expected_false(mat_val[i][j] == EDGE_VAL)) {
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
	for (i = w*h; i >= label_counter; i--) {
		j = i;
		while (parent_arr[j] != j) {
			j = parent_arr[j];
		}
		parent_arr[i] = j;
	}
	for (i = 0; i < h * w; i++) {
		edge_im[i] = parent_arr[edge_im[i]];
	}
	min_max_normalization(edge_im, w, h);

	heap_free(0,parent_arr);
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
void labeling(byte * restrict edge_im, uint32 w, uint32 h)

{
	// binary conversion

	for (int i = 0; i < W * H; i++) {
		if (edge_im[i] > 7)
			edge_im[i] = 0;
		else
			edge_im[i] = 1;
	}

	byte (*mat_val)[W] = (byte (*)[W]) edge_im;
	uint32 * restrict parent_arr = (uint32 *) heap_malloc(0, W * H * sizeof(uint32));
	if (parent_arr == NULL) {
		printf("Nije instancirana memorija\n");
		return;
	}

	for (int i = 0; i < W*H; i++)          // Everyone is it's own parent
	{
		parent_arr[i] = i;
	}
	uint32 label_counter = W*H;
	uint32 max_neighbour;


	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) {
			max_neighbour = 1;
			if (expected_false(mat_val[i][j] == EDGE_VAL)) {
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
		edge_im[i] = parent_arr[edge_im[i]];
	}
	min_max_normalization(edge_im, W, H);

	heap_free(0,parent_arr);
}
#endif
