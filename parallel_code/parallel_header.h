#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include <time.h>

#ifndef PROJECT_H_  /* Include guard */
#define PROJECT_H_

typedef struct {
  float** image_data; /* a 2D array of floats */
  int m; /* # pixels in vertical-direction */
  int n; /* # pixels in horizontal-direction */
}
image;

void allocate_image(image *u, int m, int n);
void deallocate_image(image *u);
void convert_jpeg_to_image(const unsigned char* image_chars, image *u);
void convert_image_to_jpeg(const image *u, unsigned char* image_chars);
void iso_diffusion_denoising_parallel(image *u, image *u_bar, float kappa, int iters, MPI_Comm comm_cart);
void create_n_dims(int num_procs);
void add_to_whole_image(unsigned char *sub_image, int size, int x_start, int x_stop, int y_start, int y_stop, image *whole_image);

#endif // PROJECT_H_
