#include "serial_header.h"



void allocate_image(image *u, int m, int n){
  //Allocate a 2D array.
  u-> image_data = malloc(m * sizeof *u->image_data);
  u-> image_data[0] = malloc(m*n * sizeof *u->image_data[0]);
  for (int i = 1; i < m; i ++) {
    u-> image_data[i] = &(u-> image_data[0][i*n]);
  }

  u->m = m;
  u->n = n;
  printf("Allocated Image Complete.\n" );
}



void deallocate_image(image *u){
  free(u->image_data[0]); // Must be deallocated first to avoid memory leaks.
  free(u->image_data);
 }


void convert_jpeg_to_image(const unsigned char* image_chars, image *u){
  int m = u->m;
  int n = u->n;
   /*for(int i = 0; i < n*m; i++){
     printf("image_chars[%d]: %d\n",i, image_chars[i] );
   }*/

  for(int i = 0; i<m; i++){
    for( int j = 0; j<n; j++){
      //if(i>3000)printf("%d %d, putting: %d\n",i,j, image_chars[m*i +j] );
      //printf("%d %d (%d) putting %d\n",i,j,m*i +j,image_chars[m*i +j]);
      u->image_data[i][j] = (float) image_chars[n*i + j];
      //if(i>4200)printf("%d %d, just put: %f\n",i,j, u->image_data[i][j]);
    }
  }
  printf("convert_jpeg_to_image complete.\n" );
}//end convert_jpeg_to_image()


void convert_image_to_jpeg(const image *u, unsigned char* image_chars){
  int n = u->n;
  int m = u->m;

  for(int i = 0; i<m; i++){
    for(int j = 0; j<n; j++){
      image_chars[(n*i) +j] = (int) u->image_data[i][j];
    }
  }
  printf("convert_image_to_jpeg complete.\n" );
}//end convert_image_to_jpeg()


void iso_diffusion_denoising(image *u, image *u_bar, float kappa, int iters){
  printf("ISO_DIFFUSION_DENOISING Algorithm is runnning... Iterations: %d\n",iters );
  int m = u->m;
  int n = u->n;
  float tmp;
  image* tmpImage;

  for(int itr = 0; itr < iters; itr++){
  //Iteration begin
  tmp = 0;
    for(int i = 1; i <m-1; i++){
      for(int j = 1; j<n-1; j++){
        tmp = u->image_data[i-1][j] + u->image_data[i][j-1] - 4*u->image_data[i][j] + u->image_data[i][j+1] + u->image_data[i+1][j];
        tmp *= kappa;
        tmp += u->image_data[i][j];
        u_bar->image_data[i][j] = tmp;
      }
    }
    //Edge pixels
    for(int j = 0; j<n; j++){
      u_bar->image_data[0][j] = u->image_data[0][j];
      u_bar->image_data[m-1][j] = u->image_data[m-1][j];
    }
    for(int i = 0; i<m; i++){
      u_bar->image_data[i][0] = u->image_data[i][0];
      u_bar->image_data[i][n-1] = u->image_data[i][n-1];
    }

    if(itr < iters-2){
      //Pointer swapping for all except final iteration.
      tmpImage = u;
      u = u_bar;
      u_bar = tmpImage;
    }//end if
  }//end Iteration
  printf("ISO_DIFFUSION_DENOISING Complete.\n");
}//end iso_diffusion_denoising()










//
