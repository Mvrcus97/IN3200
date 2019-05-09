#include "parallel_header.h"

void import_JPEG_file (const char* filename, unsigned char** image_chars,
  int* image_height, int* image_width,
  int* num_components);

void export_JPEG_file (const char* filename, const unsigned char* image_chars,
  int image_height, int image_width,
  int num_components, int quality);

void print_line(){ printf("----------------------------------------------------------------------------------\n");}


int main(int argc, char *argv[]){
  if (argc != 5) {
      printf("Usage: ./parallel_main 'kappa' 'iters' 'input_jpeg_filename' 'output_jpeg_filename' \n");
      printf("Example run with Mona Lisa Image:\n" );
      printf("./parallel_main 0.2 100 mona_lisa_noisy.jpg output.jpg\n" );
      return 0;
  }

  char input_filename[32], output_filename[32];
  float kappa = atof(argv[1]);
  int iters = atoi(argv[2]);
  strncpy(input_filename, argv[3], 32);
  strncpy(output_filename, argv[4], 32);

  int m, n, c;
  int my_m, my_n, my_rank, num_procs;
  image u, u_bar, whole_image;
  unsigned char *image_chars, *my_image_chars;
  char *input_jpeg_filename, *output_jpeg_filename;

  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);


  if(my_rank == 0){
    print_line();
    printf("Kappa: %f\nIters: %d\nInput File: %s\nOutput File: %s\n",kappa, iters, input_filename, output_filename);
    print_line();
    FILE* fptr;
    if ((fptr = fopen(input_filename, "r")) == NULL){
      printf("ERROR opening file: [%s]\nPlease run the program again.\n\n", input_filename);
      MPI_Finalize ();
      exit(0);
    }

    import_JPEG_file(input_filename, &image_chars, &m, &n, &c);
    printf("Successful Jpeg Import. m: %d n: %d num_components : %d\n",m,n,c );
    allocate_image (&whole_image, m, n);

  }

  MPI_Bcast (&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast (&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //Create 2D - Entropology
  //2D decomposition of the m x n pixels evenly among the MPI processes ///
  MPI_Comm comm_cart;
  int my_xstart, my_xstop, my_ystart, my_ystop, my_imax, my_jmax;
  int dim[2], period[2], reorder, my_coord[2];

  dim[0] = 2; period[0] = 0; // x direction TODO for mpi -np X
  dim[1] = 2; period[1] = 0; // y direction
  reorder=1;

  MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm_cart);
  MPI_Comm_rank(comm_cart, &my_rank);
  MPI_Cart_coords(comm_cart, my_rank, 2, my_coord);


  my_xstart = my_coord[0]*(n-2)/dim[0];
  my_xstop = (my_coord[0]+1)*(n-2)/dim[0];
  my_imax = my_xstop-my_xstart+2; // USE THSE FOR ALOCATE IMG ??

  my_ystart = my_coord[1]*(m-2)/dim[1];
  my_ystop = (my_coord[1]+1)*(m-2)/dim[1];
  my_jmax = my_ystop-my_ystart+2;

  //printf("RANK: %d, pos: (%d, %d)  x: (%d, %d)  y: (%d, %d) iMax: %d jMax: %d\n", my_rank, my_coord[0], my_coord[1], my_xstart, my_xstop, my_ystart, my_ystop, my_imax, my_jmax );


  printf("%d m %d: n %d: \n",my_rank, my_jmax, my_imax );
  //TODO CAUSED ERROR: WAS 0, 0 ON ALOCATE...
  allocate_image (&u, my_jmax, my_imax);
  allocate_image (&u_bar, my_jmax, my_imax);

  /* each process asks process 0 for a partitioned region */
  /* of image_chars and copy the values into u */
  MPI_Status status;
  int sub_image_size = n*m/num_procs;
  my_image_chars = malloc(sub_image_size * sizeof(int));

  if(my_rank == 0){
    //Master proccess: Request indicies, create sub_image and send it.
    int to_send_idx[4];
    unsigned char *sub_image = malloc(sub_image_size * sizeof(int));
    int curr_y_start, curr_y_stop, curr_x_start, curr_x_stop, idx_ctr;

    /*For each proccess: Recieve current process' region
    and create sub_image, and send it. */
    for(int i = 1; i < num_procs; i++){
      idx_ctr = 0;
      MPI_Recv(&to_send_idx, 4 , MPI_INT, i, 0, MPI_COMM_WORLD, &status);
      curr_x_start = to_send_idx[0]; curr_x_stop = to_send_idx[1];
      curr_y_start = to_send_idx[2]; curr_y_stop = to_send_idx[3];
      printf("FOUND: curr x %d %d , curr y %d %d\n",curr_x_start, curr_x_stop, curr_y_start, curr_y_stop );
      for(int j = curr_y_start; j<curr_y_stop; j++){
        for(int k = curr_x_start; k<curr_x_stop; k++){
          sub_image[idx_ctr] = image_chars[(n*j) +k];
          idx_ctr++;
        }//end for k
      }//end for j

      MPI_Send(sub_image, sub_image_size, MPI_INT, i, 0, MPI_COMM_WORLD);
    }//end for i

    //Finally create sub_image for master.
    idx_ctr = 0;
    for(int j = my_ystart; j<my_ystop; j++){
      for(int k = my_xstart; k<my_xstop; k++){
        my_image_chars[idx_ctr] = image_chars[(n*j) +k];
        idx_ctr++;
      }//end for k
    }//end for j
  }//end Master

  if(my_rank != 0){
    // Tell master which indicies are mine, and recieve a corresponding sub_image.
    int my_idx[4];
    my_idx[0] = my_xstart; my_idx[1] = my_xstop;
    my_idx[2] = my_ystart; my_idx[3] = my_ystop;
    MPI_Send(&my_idx, 4, MPI_INT, 0,0, MPI_COMM_WORLD);
    //printf("Waiting to recieve: %d\n",sub_image_size );
    MPI_Recv(my_image_chars, sub_image_size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    //printf("I RECIEVED! %d: %d \n",my_rank, my_image_chars[100]);
  }


  convert_jpeg_to_image (my_image_chars, &u);
  iso_diffusion_denoising_parallel(&u, &u_bar, kappa, iters, comm_cart);
  //TEST TEST

  convert_image_to_jpeg (&u_bar, my_image_chars);

  if(my_rank == 0){
    printf("y: %d, x: %d,\n", my_ystop-my_ystart, my_xstop-my_xstart);
    export_JPEG_file("output0.jpg", my_image_chars, my_ystop-my_ystart, my_xstop-my_xstart, 1, 75);
  }
  if(my_rank == 1){
    printf("y: %d, x: %d,\n", my_ystop-my_ystart, my_xstop-my_xstart);
    export_JPEG_file("output1.jpg", my_image_chars, my_ystop-my_ystart, my_xstop-my_xstart, 1, 75);
  }
  if(my_rank == 2){
    printf("y: %d, x: %d,\n", my_ystop-my_ystart, my_xstop-my_xstart);
    export_JPEG_file("output2.jpg", my_image_chars, my_ystop-my_ystart, my_xstop-my_xstart, 1, 75);
  }
  if(my_rank == 3){
    printf("y: %d, x: %d,\n", my_ystop-my_ystart, my_xstop-my_xstart);
    export_JPEG_file("output3.jpg", my_image_chars, my_ystop-my_ystart, my_xstop-my_xstart, 1, 75);
  }


  //TEST STOP



  /* each process sends its resulting content of u_bar to process 0 */
  /* process 0 receives from each process incoming values and */
  /* copy them into the designated region of struct whole_image */
  /* ... */
  /*if (my_rank==0) {
    convert_image_to_jpeg(&whole_image, image_chars);
    export_JPEG_file(output_filename, image_chars, m, n, c, 75);
    deallocate_image (&whole_image);
  }
*/
  deallocate_image (&u);
  deallocate_image (&u_bar);
  MPI_Finalize ();
  return 0;
}
