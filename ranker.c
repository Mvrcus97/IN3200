#include "exam_header.h"


/*  This is the actual function called to calculate the PageRanks.
*   Iterativly update the scores of all webpages. Stop when either the scores have converged,
*   or all iterations are complete.
*/
void PageRank_iterations(int nodes, int edges, double d, double eps, int* row_ptr, int* col_idx, double* val, int* danglings, int itr, double** scores){
  double* x = malloc(nodes*sizeof(double*));
  double* x_new = malloc(nodes*sizeof(double*));
  double* tmp;
  double baseVal = 1.0/nodes;
  int i;

  //Initialize x.
  for(i = 0; i < nodes; i++){
    x[i] = baseVal;
  }

  double sum = 0.0, totA = 0.0, totW = 0.0;
  double w, start, end, tmp_sum, max, curr_val;
  int done, val_idx, curr_amount, j;
  int stop = 0;
  //Go through itr iterations and perform the calculation.

  #pragma omp parallel private(i)
  {
  for( i = 0; i < itr; i++){
    //Step 1: Calculate W.
    #pragma omp single
    {
      start = omp_get_wtime(); // Returns time in seconds.
      sum = 0.0;
    }

      #pragma omp for reduction(+:sum)
      for( int i = 0; i <nodes; i++){
        if(danglings[i]){
          sum += x[i];
        }
      }

    #pragma omp single
    {
      w = ( (1 - d) + (d * sum) );
      w = w/nodes;
    }

    #pragma omp for
      for( int i = 0; i < nodes; i++){
        x_new[i] = w;
      }
      #pragma omp single
      {
       end = omp_get_wtime();
       totW += end - start;
      }

    //Step 2: Calculate Ax.
    #pragma omp single
    {
      start = omp_get_wtime(); // Returns time in seconds.
    }

    #pragma omp for private(tmp_sum, curr_amount, val_idx, j)
    for(int k = 0; k < nodes+1; k++){
      tmp_sum = 0.0;
      curr_amount = row_ptr[k+1] - row_ptr[k];
      val_idx = row_ptr[k];
      //printf("[ID: %d] curr_amount: %d, val_idx: %d\n",omp_get_thread_num(), curr_amount, val_idx );
      for(j = 0; j<curr_amount; j++){
        tmp_sum += val[val_idx] * x[col_idx[val_idx]];
        val_idx ++;
      }
      x_new[k] += tmp_sum * d;
    }

    #pragma omp single
    {
      end = omp_get_wtime();
      totA  += end - start;
    //Step 3: pointer swapping

    tmp = x_new;
    x_new = x;
    x = tmp;

    max = 0.0;
  }

    //Step 4: Check if converged
  #pragma omp for private(curr_val) reduction(max:max)
  for(int k = 0; k<nodes; k++){
    if(( curr_val= fabs(x[i] - x_new[i])) > max){
      max = curr_val;
    }
  }

  #pragma omp single
  {
    //printf("MAX DIFF: %lf. eps: %lf\n",max, eps );
    if(max < eps) stop = 1;
  }
  if(stop)break;


  }//End for iteration
  #pragma omp single
  {
    done = i;
  }
  }//end Parallel

  printf("\nConverged after %d iterations. Results:: \n", done);

  printf("Time W: %lf.  Time Ax: %lf\n",totW, totA );
  //for(i = 0; i < nodes; i++){
    //printf("x[%d]:%f  ",i, x[i] );
  //}
  printf("\n\n" );



  //Finally update output to the correct webpage ranks.
  *scores = x;
}//end PageRank_iterations




/*  This functions calculates part 1/2 of the iterative procedure.
*   Updates x_new to contain part 1 of 2 of the iterative procedure.
*/
void W_funcSeq(double* x, double* x_new, int* danglings, int nodes, double d){
  double sum = 0.0;
  double w;

    for( int i = 0; i <nodes; i++){
      if(danglings[i]){
        sum += x[i];
      }
    }

  w = (( (1 - d) + (d * sum) )/nodes);
    for( int i = 0; i < nodes; i++){
      x_new[i] = w;
    }
}//end W_func

/*  This functions calculates part 2/2 of the iterative procedure.
*   Updates x_new to contain the final result of the iterative procedure.
*/
void Ax(int nodes, double d, int* row_ptr, int* col_idx, double* val, double* x, double* x_new){
  int curr_amount = 0;
  double tmp_sum = 0.0;
  int val_idx = 0;

  for( int i = 0; i < nodes+1; i ++){
    curr_amount = row_ptr[i+1] - row_ptr[i];
    for(int j = 0; j < curr_amount; j++){
      tmp_sum += val[val_idx] * x[col_idx[val_idx]];
      val_idx ++;
    }
    x_new[i] += tmp_sum * d;
    tmp_sum = 0.0;
  }
}//end Ax.

/* This fucntion calculates the biggest difference between x[i] and x_new[i].
*If this difference is less than the defined epsilon, return true(1).
Else, return false(0).
*/
int early_stopping(int nodes, double* x, double* x_new, double eps){
  int res = 0;
  double max = 0.0;
  double curr_val;

  for( int i = 0; i < nodes; i ++){
    if((curr_val = fabs(x[i] - x_new[i])) > max){
      max = curr_val;
    }
  }

  if(max < eps) res = 1;
  return res;
}


void check_col(int nodes, int edges, int* col_idx, double* val){
  double sum = 0.0;
  for(int i = 0; i<nodes; i++){
    for( int j = 0; j<=edges; j++){
      if(col_idx[j] == i) sum += val[j];
    }
  if( (sum != 0) && (sum != 1)) printf("col: %d. Val: %lf\n",i, sum );
  sum = 0.0;
  }
}
