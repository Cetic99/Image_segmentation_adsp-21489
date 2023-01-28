#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <time.h>
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
//#define UINT_MAX                4294967295
#define EDGE_VAL                0
#define NUM_LABELS              50000

//compiling
#define LABELING_2
#define CONVOLUTION_2
#define EDGE_2

typedef unsigned int uint32;
typedef signed int int32;
typedef short int16;
typedef unsigned char byte;

typedef enum im_type
{
    GRAY,
    COLORED
} type;

#define SIZE 256
typedef struct rgb{
    byte r,g,b;
}RGB;
RGB colormap[SIZE] = {0};

void create_colormap()
{
    srand(time(0));
    for(int i= 1; i< SIZE; i++){
        colormap[i].r = rand();
        colormap[i].g = rand();
        colormap[i].b = rand();
    }
}

void ReadImage(const char *fileName, byte **pixels, uint32 *width, uint32 *height, uint32 *bytesPerPixel)
{
    FILE *imageFile = fopen(fileName, "rb");
    uint32 dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);
    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);
    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);
    int16 bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    *bytesPerPixel = ((uint32)bitsPerPixel) / 8;

    int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f)) * (*bytesPerPixel);
    int unpaddedRowSize = (*width) * (*bytesPerPixel);
    int totalSize = unpaddedRowSize * (*height);
    *pixels = (byte *)malloc(totalSize);
    int i = 0;
    byte *currentRowPointer = *pixels + ((*height - 1) * unpaddedRowSize);
    for (i = 0; i < *height; i++)
    {
        fseek(imageFile, dataOffset + (i * paddedRowSize), SEEK_SET);
        fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
        currentRowPointer -= unpaddedRowSize;
    }

    fclose(imageFile);
    // printf("Slika ucitana!\n");
}

void WriteImage(const char *fileName, byte *pixels, uint32 width, uint32 height,type t)
{
    uint32 bytesPerPixel;
    if(t == GRAY)
        bytesPerPixel = 1;
    else
        bytesPerPixel = 3;
    FILE *outputFile = fopen(fileName, "wb");
    //*****HEADER************//
    const char *BM = "BM";
    fwrite(&BM[0], 1, 1, outputFile);
    fwrite(&BM[1], 1, 1, outputFile);
    int paddedRowSize = (int)(4 * ceil((float)width / 4.0f)) * bytesPerPixel;
    uint32 fileSize = paddedRowSize * height + HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&fileSize, 4, 1, outputFile);
    uint32 reserved = 0x0000;
    fwrite(&reserved, 4, 1, outputFile);
    uint32 dataOffset = HEADER_SIZE + INFO_HEADER_SIZE;
    if (t == GRAY)
    {
        dataOffset += 4 * 256;
    }
    fwrite(&dataOffset, 4, 1, outputFile);

    //*******INFO*HEADER******//
    uint32 infoHeaderSize = INFO_HEADER_SIZE;
    fwrite(&infoHeaderSize, 4, 1, outputFile);
    fwrite(&width, 4, 1, outputFile);
    fwrite(&height, 4, 1, outputFile);
    int16 planes = 1; // always 1
    fwrite(&planes, 2, 1, outputFile);
    int16 bitsPerPixel = bytesPerPixel * 8;
    fwrite(&bitsPerPixel, 2, 1, outputFile);
    // write compression
    uint32 compression = NO_COMPRESION;
    fwrite(&compression, 4, 1, outputFile);
    // write image size (in bytes)
    uint32 imageSize;
    uint32 resolutionX;
    uint32 resolutionY;
    if(t == GRAY){
        imageSize = 0;
        resolutionX = 0; // 11811; //300 dpi
        resolutionY = 0; // 11811; //300 dpi
    }
    else{
        imageSize = width*height*bytesPerPixel;
        resolutionX = 0;
        resolutionY = 0;
    }
    fwrite(&imageSize, 4, 1, outputFile);
    fwrite(&resolutionX, 4, 1, outputFile);
    fwrite(&resolutionY, 4, 1, outputFile);
    uint32 colorsUsed = MAX_NUMBER_OF_COLORS;
    fwrite(&colorsUsed, 4, 1, outputFile);
    uint32 importantColors = ALL_COLORS_REQUIRED;
    fwrite(&importantColors, 4, 1, outputFile);
    int unpaddedRowSize = width * bytesPerPixel;
    if (t == GRAY)
    {
        int zero = 0;
        for (int i = 0; i < 256; i++)
        {
            fwrite(&i, 1, 1, outputFile);
            fwrite(&i, 1, 1, outputFile);
            fwrite(&i, 1, 1, outputFile);
            fwrite(&zero, 1, 1, outputFile);
        }
    }

    for (int i = 0; i < height; i++)
    {
        // printf("Upis reda\n");
        int pixelOffset = ((height - i) - 1) * unpaddedRowSize;
        fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
    }
    fclose(outputFile);
    // printf("Slika sacuvana!\n");
}

byte *to_gray(uint32 height, uint32 width, byte *pixels, uint32 *bytesPerPixel)
{
    *bytesPerPixel = 1;
    byte *gray_pixels = malloc(height * width);
    int pix_position;
    int num_iter = height*width;
    #pragma omp parallel for
    for (int i = 0; i < num_iter; i++)
    {
        pix_position = i * 3;
        //                 ------RED--------                 --------GREEN--------               ------BLUE-------
        gray_pixels[i] = pixels[pix_position + 2] * 0.299 + pixels[pix_position + 1] * 0.587 + pixels[pix_position] * 0.114;
    }
    return gray_pixels;
}

#ifdef CONVOLUTION_1
static inline uint32 convolution(byte *pixels, uint32 kernel[3][3], uint32 row, uint32 column, uint32 width)
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
#ifdef CONVOLUTION_2
static inline uint32 convolution(byte *pixels, uint32 kernel[3][3], uint32 row, uint32 column, uint32 width)
{
    byte(*pix_mat)[width] = (byte(*)[width])pixels;
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

void min_max_normalization(uint32 *pixels, uint32 width, uint32 height)
{
    uint32 min = INT_MAX, max = 0, i;
    uint32 length = height*width;
    #pragma omp parallel for
    for (i = 0; i < length; i++)
    {
            if (pixels[i] < min)
            {
                min = pixels[i];
            }
            if (pixels[i] > max)
            {
                max = pixels[i];
            }
    }
    double sub = max - min;
    double ratio;
    #pragma omp parallel for
    for (i = 0; i < length; i++)
    {
            ratio = (double)(pixels[i] - min) / sub;
            pixels[i] = ratio * 255;
    }
    // for (i = 0; i < length; i++)
    // {
    //         double ratio = (double)(pixels[i] - min) / (max - min);
    //         pixels[i] = ratio * 255;
    // }
    // uint32 trh = (max-min) * 0.05 + min;
    // for (i = 0; i < length; i++)
    // {

    //     if (pixels[i] > trh)
    //     {
    //         pixels[i] = 255;
    //     }    
    //     else
    //     {
    //         pixels[i] = 0;
    //     }
    // }

    // printf("Normalized!\n");
}

#ifdef EDGE_1
void sobel_edge_detector(byte *pixels, byte **out_pixels, uint32 width, uint32 height)
{
    uint32 i, j, gx, gy;

    byte *edged_pixels_arr = (byte *)malloc(width * height);                            // creating array of bytes
    *out_pixels = edged_pixels_arr;                                                     // assigning array to output pointer

    uint32 *edged_pixels_arr_uint32 = (uint32 *)malloc(width * height * sizeof(uint32));    // creating array of uint32
    uint32(*edged_pixels_mat_uint32)[width] = (uint32(*)[width])edged_pixels_arr_uint32;    // converting array to matrix

    uint32 mx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};
    uint32 my[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    for (i = 1; i < height - 2; i++)
    {
        for (j = 1; j < width - 2; j++)
        {
            gx = convolution(pixels, mx, i, j, width);
            gy = convolution(pixels, my, i, j, width);
            edged_pixels_mat_uint32[i][j] = sqrt(gx * gx + gy * gy);
        }
        // printf("Convolved!\n");
    }
    // printf("Done with convolution!\n");
    min_max_normalization(edged_pixels_arr_uint32, width, height);
    for (i = 0; i < height * width; i++)
    {
        edged_pixels_arr[i] = edged_pixels_arr_uint32[i];                //converting from 32bit to 8bit
        // printf("%d",edged_pixels_arr[i]);
    }
}
#endif

#ifdef EDGE_2
void sobel_edge_detector(byte *pixels, byte **out_pixels, uint32 width, uint32 height)
{
    uint32 i, j, gx, gy;

    byte *edged_pixels_arr = (byte *)malloc(width * height);                            // creating array of bytes
    *out_pixels = edged_pixels_arr;                                                     // assigning array to output pointer

    uint32 *edged_pixels_arr_uint32 = (uint32 *)malloc(width * height * sizeof(uint32));    // creating array of uint32
    uint32(*edged_pixels_mat_uint32)[width] = (uint32(*)[width])edged_pixels_arr_uint32;    // converting array to matrix

    uint32 mx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};
    uint32 my[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    for (i = 1; i < height - 2; i++)
    {
        for (j = 1; j < width - 2; j++)
        {
            gx = convolution(pixels, mx, i, j, width);
            gy = convolution(pixels, my, i, j, width);
            edged_pixels_mat_uint32[i][j] = sqrt(gx * gx + gy * gy);
        }
        // printf("Convolved!\n");
    }
    // printf("Done with convolution!\n");
    min_max_normalization(edged_pixels_arr_uint32, width, height);
    for (i = 0; i < height * width; i++)
    {
        edged_pixels_arr[i] = edged_pixels_arr_uint32[i];                //converting from 32bit to 8bit
        // printf("%d",edged_pixels_arr[i]);
    }
}
#endif

#ifdef LABELING_1
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
    uint32 *parent = malloc(w*h*4);
    for(int i = 0;i<w*h;i++)                // Everyone is it's own parent
    {
        parent[i] = i;
    }
    uint32 label_counter = 1;
    uint32 min_neighbour;
    uint32 *mat_arr = malloc(w*h*4);
    for( int i = 0; i< w*h; i++){
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
            mat_val[i][j] = min_neighbour;        //assigning this pixel its label value
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
        while(parent[j] != j){
            j = parent[j];
        }
        parent[i] = j;
    }
    for(i = 0 ; i< h ; i++)
    {
        for(j = 0; j < w; j++)
        {
            mat_val[i][j] = parent[mat_val[i][j]];
        }
    }
    min_max_normalization(mat_arr, w, h);
    for(i = 0 ; i<w*h; i++){
        edge_im[i] = mat_arr[i]; 
    }

}
#endif

#ifdef LABELING_2
void labeling(byte * edge_im, uint32 w,uint32 h)

{
        // binary conversion
    for(int i =0; i<w*h;i++)
    {
        if(edge_im[i] > 10)
            edge_im[i] = 0;
        else
            edge_im[i] = 1;
    }
    /*
        Maybe it is better to make new matrix with values of edges UINT_MAX
        With this it will be possible to remove EDGE_VAL in if statement
        WAITING FOR NEXT ITERATION!!!!!!!
    */
    byte (*edge_mat)[w] = (byte(*)[w])edge_im;
    uint32 *parent = malloc(w*h*4);
    for(int i = 0;i<NUM_LABELS;i++)                // Everyone is it's own parent
    {
        parent[i] = i;
    }
    uint32 label_counter = NUM_LABELS;
    uint32 max_neighbour;
    uint32 *mat_arr = malloc(w*h*4);
    for( int i = 0; i< w*h; i++){
        mat_arr[i] = edge_im[i];
    }
    uint32 (*mat_val)[w] = (uint32(*)[w])mat_arr;
    
    for(int i = 1; i< h-1; i++)
    {
        for(int j = 1; j < w-1; j++)
        {
            max_neighbour = 1;
            if(edge_mat[i][j] == EDGE_VAL)
            {
                continue;
            }
            if (mat_val[i-1][j-1] > max_neighbour)
            {
                max_neighbour = mat_val[i-1][j-1];
            }
            if (mat_val[i-1][j] > max_neighbour)
            {
                max_neighbour = mat_val[i-1][j];
            }
            if (mat_val[i-1][j+1] > max_neighbour)
            {
                max_neighbour = mat_val[i-1][j+1];
            }
            if (mat_val[i][j-1] > max_neighbour)
            {
                max_neighbour = mat_val[i][j-1];
            }
            if(max_neighbour == 1)
            {
                label_counter--;
                max_neighbour = label_counter;
            }
            mat_val[i][j] = max_neighbour;        //assigning this pixel its label value
            if (mat_val[i-1][j+1] < max_neighbour)
            {
                parent[mat_val[i-1][j+1]] = max_neighbour;
            }
            if (mat_val[i-1][j] < max_neighbour)
            {
                parent[mat_val[i-1][j]] = max_neighbour;
            }
        }
    }
    int i = 0;
    int j = 0;
    for(i = NUM_LABELS; i >= label_counter; i--)
    {
        j = i;
        while(parent[j] != j){
            j = parent[j];
        }
        parent[i] = j;
    }
    #pragma omp parallel for
    for(i = 0 ; i< h*w ; i++)
    {
        mat_arr[i] = parent[mat_arr[i]];
    }
    min_max_normalization(mat_arr, w, h);
    #pragma omp parallel for
    for(i = 0 ; i<w*h; i++){
        edge_im[i] = mat_arr[i]; 
    }

}
#endif
void colorImage(byte * labeled_im, byte* colored_im, uint32 w, uint32 h)
{
    uint32 num_iter = w*h;
    uint32 counter = 1;
    #pragma omp parallel for
    for(int i= 0;i<num_iter;i++){
        counter = i*3;
        colored_im[counter] = colormap[labeled_im[i]].b;
        colored_im[counter+1] = colormap[labeled_im[i]].g;
        colored_im[counter+2] = colormap[labeled_im[i]].r;
    }
}
int main()
{
    clock_t start, end,start_program;
    start_program = clock();
    byte *pixels;
    byte *gray_pix_arr;
    byte *edged_pix_array;
    uint32 width;
    uint32 height;
    uint32 bytesPerPixel;

    // reading Image into pixels
    start = clock();
    ReadImage("Ja.bmp", &pixels, &width, &height, &bytesPerPixel);
    end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("READING IMAGE: %fs\n", time_taken);
    WriteImage("loop-back.bmp", pixels, width, height, COLORED);

    //converting to gray
    start = clock();
    gray_pix_arr = to_gray(height, width, pixels, &bytesPerPixel);
    end = clock();
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("GRAYSCALE conversion: %fs\n", time_taken);
    WriteImage("Grayscale.bmp", gray_pix_arr, width, height, GRAY);

    // detecting edges
    start = clock();
    sobel_edge_detector(gray_pix_arr, &edged_pix_array, width, height);
    end = clock();
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    WriteImage("Edged.bmp", edged_pix_array, width, height, GRAY);
    printf("EDGE detection: %fs\n", time_taken);

    //labeling----coding image
    start = clock();
    labeling(edged_pix_array,width,height);
    end = clock();
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("LABELING: %fs\n", time_taken);

    create_colormap();

    start = clock();
    colorImage(edged_pix_array,pixels,width,height);
    end = clock();
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("PAINTING: %fs\n", time_taken);

    //writing image to file
    start = clock();
    WriteImage("Output.bmp", pixels, width, height, COLORED);
    end = clock();
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("WRITING IMAGE: %fs\n", time_taken);

    time_taken = (double)(end - start_program) / CLOCKS_PER_SEC;
    printf("PROGRAM: %fs\n", time_taken);
    // freeing memory
    free(pixels);
    free(gray_pix_arr);
    free(edged_pix_array);
    return 0;
}