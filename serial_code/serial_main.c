#include "serial_header.h"
//#include "functions.c"

void import_JPEG_file (const char* filename, unsigned char** image_chars,
  int* image_height, int* image_width,
  int* num_components);

void export_JPEG_file (const char* filename, const unsigned char* image_chars,
  int image_height, int image_width,
  int num_components, int quality);

void print_line(){ printf("--------------------------------------------------------------------------------------------\n");}





int main(int argc, char *argv[]){
  if (argc != 5) {
      printf("Usage: ./serial_main 'kappa' 'iters' 'input_jpeg_filename' 'output_jpeg_filename' \n");
      printf("Example run with Mona Lisa Image:\n" );
      printf("./serial_main 0.2 100 mona_lisa_noisy.jpg output.jpg\n" );
      return 0;
  }

  char input_filename[32], output_filename[32];
  float kappa = atof(argv[1]);
  int iters = atoi(argv[2]);
  strncpy(input_filename, argv[3], 32);
  strncpy(output_filename, argv[4], 32);
  print_line();
  printf("Kappa: %f\nIters: %d\nInput File: %s\nOutput File: %s\n",kappa, iters, input_filename, output_filename);

  FILE* fptr;
  if ((fptr = fopen(input_filename, "r")) == NULL){
    printf("ERROR opening file: [%s]\nPlease run the program again.\n\n", input_filename);
    exit(1);
   }

  int m, n, c;
  image u, u_bar;
  unsigned char *image_chars;

  import_JPEG_file(input_filename, &image_chars, &m, &n, &c);
  print_line();
  printf("                             Import Successful.\n        Vertical pixels: %d, Horizontal pixels: %d, Num components: %d\n",
	 m,n, c);
  print_line();


  allocate_image (&u, m, n);
  allocate_image (&u_bar, m, n);
  convert_jpeg_to_image (image_chars, &u);
  iso_diffusion_denoising (&u, &u_bar, kappa, iters);
  convert_image_to_jpeg (&u_bar, image_chars);
  export_JPEG_file(output_filename, image_chars, m, n, c, 75);
  deallocate_image (&u);
  deallocate_image (&u_bar);
  return 0;
}
