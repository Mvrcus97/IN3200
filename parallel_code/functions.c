#include "parallel_header.h"



void allocate_image(image *u, int m, int n){
  //Allocate a 2D array.
  u-> image_data = malloc(m * sizeof *u->image_data);
  u-> image_data[0] = malloc(m*n * sizeof *u->image_data[0]);
  for (int i = 1; i < m; i ++) {
    u-> image_data[i] = &(u-> image_data[0][i*n]);
  }

  u->m = m;
  u->n = n;
  //printf("Allocated Image Complete.\n" );
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
      //if(i>2000)printf("%d %d, just put: %f\n",i,j, u->image_data[i][j]);
    }
  }
  //printf("convert_jpeg_to_image complete.\n" );
  printf("m: %d, n: %d\n",m,n );
}//end convert_jpeg_to_image()


void convert_image_to_jpeg(const image *u, unsigned char* image_chars){
  int n = u->n;
  int m = u->m;

  for(int i = 0; i<m; i++){
    for(int j = 0; j<n; j++){
      image_chars[(n*i) +j] = (int) u->image_data[i][j];
    }
  }
  //printf("convert_image_to_jpeg complete.\n" );
}//end convert_image_to_jpeg()


void iso_diffusion_denoising_parallel(image *u, image *u_bar, float kappa, int iters, MPI_Comm comm_cart){
  //printf("ISO_DIFFUSION_DENOISING Algorithm is runnning... Iterations: %d\n",iters );
  int m = u->m;
  int n = u->n;
  float tmp;
  image* tmpImage;

  int my_rank;
  int my_coord[2];
  int idx_ctr;
  int rankS, rankD;
  MPI_Request reqs[4];
  MPI_Status stats[4];

  int *right_halo = malloc(m*sizeof(int)); int *left_halo = malloc(m*sizeof(int));
  int *top_halo   = malloc(n*sizeof(int)); int *bot_halo  = malloc(n*sizeof(int));
  int right_neighboar, left_neighboar, top_neighboar, bot_neighboar;
  int right_exists = 0; int left_exists = 0;
  int top_exists   = 0; int  bot_exists = 0;


  //get coordinates and rank.
  MPI_Comm_rank(comm_cart, &my_rank);
  //printf("%d, m: %d, n: %d\n",my_rank, m, n );
  MPI_Cart_coords(comm_cart, my_rank, 2, my_coord);
  //printf("rank: %d  pos (%d, %d)\n",my_rank, my_coord[0], my_coord[1] );


//------------------ITERATION BEGIN ---------------------------------------
  for(int itr = 0; itr < iters; itr++){

    //---------------------VERTICAL COMMUNICATION -----------------------------------
    MPI_Barrier(comm_cart);
    MPI_Cart_shift(comm_cart, 1, 1, &rankS, &rankD);
    //printf("rank: %d, rank_source: %d, rank_dest: %d\n",my_rank, rankS, rankD );

    if(rankD != MPI_PROC_NULL){
      //There is someone to the right. We want to send our layer to the right, and recieve from  from our right.
      right_exists = 1;
      realloc(right_halo, m*sizeof(int));
      idx_ctr = 0;
      for(int i = 0; i<m; i++){
        //printf("RIGHT HALO: %f\n",u->image_data[i*n][n-2]);
        right_halo[idx_ctr] = u->image_data[i][n]; //TODO CRASH HERE 18:54 ONSDAG
        idx_ctr ++;
      }
      MPI_Isend(right_halo, m, MPI_INT, rankD, 0, comm_cart, &reqs[1]);
      realloc(left_halo, m*sizeof(int));
      //printf("%d, Recieve RIGHT from %d\n", my_rank, rankD );
      MPI_Irecv(left_halo, m, MPI_INT, rankD, 0, comm_cart, &reqs[0]);
    }

    if(rankS != MPI_PROC_NULL){
      //There is someone to the left. Send and recieve left halo.
      left_exists = 1;
      realloc(left_halo, m*sizeof(int));
      idx_ctr = 0;
      for(int i = 0; i<m; i++){
        left_halo[idx_ctr] = u->image_data[i][0];
        idx_ctr ++;
      }
      MPI_Isend(left_halo, m, MPI_INT, rankS, 0, comm_cart, &reqs[1]);
      realloc(right_halo, m*sizeof(int));
      MPI_Irecv(right_halo, m, MPI_INT, rankS, 0, comm_cart, &reqs[0]);
    }
    printf("VERTICAL COMMUNICATION DONE: %d\n", my_rank);

    //---------------------HORIZONTAL COMMUNICATION -----------------------------------

    MPI_Barrier(comm_cart); // TEMP: TODO

    MPI_Cart_shift(comm_cart, 0, 1, &rankS, &rankD);
    //printf("\n" );
    //printf("rank: %d, rank_source: %d, rank_dest: %d\n",my_rank, rankS, rankD );

    if(rankS != MPI_PROC_NULL){
      //There is someone below. We want to send our halo-layers down, and recieve halo-layers from bottom.
      //printf("%d, begin below_swap\n",my_rank );
      bot_exists = 1;
      MPI_Isend(u->image_data[0], n, MPI_INT, rankS, 0, comm_cart, &reqs[2]);
      realloc(bot_halo, n*sizeof(int));
      MPI_Irecv(bot_halo, n, MPI_INT, rankS, 0, comm_cart, &reqs[3]);
    }

    if(rankD != MPI_PROC_NULL){
      //There is someone above us.  We want to send our halo-layers up, and recieve halo-layers from top.
      printf("%d, begin below_swap\n",my_rank );
      top_exists = 1;
      MPI_Isend(u->image_data[m], n, MPI_INT, rankD, 0, comm_cart, &reqs[2]);
      printf("%d, done with send below\n",my_rank );
      realloc(top_halo, n*sizeof(int));
      MPI_Irecv(top_halo, n, MPI_INT, rankD, 0, comm_cart, &reqs[2]);
    }


  printf("COMMUNICATION DONE:  %d\n",my_rank);


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
      if(top_exists) u_bar->image_data[0][j] = top_halo[j];
      if(bot_exists) u_bar->image_data[m-1][j] = bot_halo[j];
    }

    for(int i = 0; i<m; i++){
      if(left_exists) u_bar->image_data[i][0] = left_halo[i];
      if(right_exists)u_bar->image_data[i][n-1] = right_halo[i];
    }

    if(itr < iters-2){
      //Pointer swapping for all except final iteration.
      tmpImage = u;
      u = u_bar;
      u_bar = tmpImage;
    }//end if
  }//end Iteration
  //printf("ISO_DIFFUSION_DENOISING Complete.\n");
}//end iso_diffusion_denoising()


void copySendBuf0 (int **array, int *sbuf0, int my_imax, int my_jmax){
  for (int j=1; j<my_jmax-1; j++){
    sbuf0[j-1] = array[j][1];
  }
}

void copyRecvBuf0 (int **array, int *rbuf0, int my_imax, int my_jmax){
  for (int j=1; j<my_jmax-1; j++){
    array[j][0] = rbuf0[j-1];
  }
}






//
