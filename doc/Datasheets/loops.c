/*****************************************************************************
 * loops.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "loops.h"

#include <stdio.h>
#include <cycle_count.h>

cycle_t start_count;
cycle_t final_count;

#define SIZE 100
int a[SIZE];
int b[SIZE];
/* Adding pm keyword to enforce dual memory access */
/* int pm b[SIZE]; */
int c[SIZE];

/* First illustration: No optimization */
void mul_vecs(int *out, int * input1, int * input2)
{
	int i;

	for (i = 0; i < SIZE; i++)
		out[i] = input1[i] * input2[i];
}

/* First illustration: Adding restrict keyword to enforce software pipelining */
/*
void mul_vecs(int *out, int * restrict input1, int * restrict input2)
{
	int i;

	for (i = 0; i < SIZE; i++)
		out[i] = input1[i] * input2[i];
}
*/

/* First illustration: Adding pm keyword to enforce dual memory access */
/*
void mul_vecs(int *out, int * restrict input1, pm int * restrict input2)
{
	int i;

	for (i = 0; i < SIZE; i++)
		out[i] = input1[i] * input2[i];
}
*/

/* Second illustration: No optimization */
void mul(int *out, int *input1, int *input2)
{
	*out = *input1 * *input2;
}

/* Second illustration: Adding inline keyword to enforce software pipelining */
/*
inline void mul(int *out, int *input1, int *input2)
{
	*out = *input1 * *input2;
}
*/

/* Second illustration: Adding pm keyword to enforce dual memory access */
/*
inline void mul(int *out, int *input1, pm int *input2)
{
	*out = *input1 * *input2;
}
*/

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";

int main(int argc, char *argv[])
{
	/* Second illustration (iteration counter)
	int i;
	*/

	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	/* Begin adding your custom code here */

	START_CYCLE_COUNT(start_count);

	/* First illustration: Function call */
	mul_vecs(c, a, b);

	/* Second illustration: Function call from a loop */
	/*
	for (i = 0; i < SIZE; i++)
		mul(&c[i], &a[i], &b[i]);
	*/

	STOP_CYCLE_COUNT(final_count, start_count);
	PRINT_CYCLES("Broj ciklusa: ", final_count);

	return 0;
}
