#include "exam_header.h"


/*  This is the actual function called to calculate the PageRanks.
*   Iterativly update the scores of all webpages. Stop when either the scores have converged,
*   or all iterations are complete.
*/
void PageRank_iterations(int nodes, double d, double eps, int* row_ptr, int* col_idx, double* val, int* danglings, int itr, double** scores){
  double* x = malloc(nodes*sizeof(*x));
  double* x_new = malloc(nodes*sizeof(*x));
  double* tmp;
  double baseVal = 1.0/nodes;
  int i;

  //Initialize x.
  for(i = 0; i < nodes; i++){
    x[i] = baseVal;
  }

  double sum = 0.0;
  //Go through itr iterations and perform the calculation.
  #pragma omp parallel
  {
  for( i = 0; i < itr; i++){
    //Step 1: Calculate W.
    W_funcSeq(x, x_new, danglings, nodes, d);

    //Step 2: Calculate Ax.
    Ax(nodes, d, row_ptr, col_idx, val, x, x_new);

    //Step 3: pointer swapping
    tmp = x_new;
    x_new = x;
    x = tmp;

    //Step 4: Check if converged
    if(early_stopping(nodes, x, x_new, eps)) break;

    //Step 5: Print current iteration.
    //if( i%(itr/nodes) == 0){
      //printf("\nIteration %d: \n", i );
    //  for( int i = 0; i < nodes; i++){
      //  printf("x[%d]:%f  ",i, x[i] );
    //  }
    //  printf("\n\n" );
    //}
  }//End for iteration
  }//end Parallel

  printf("\nConverged after %d iterations. Results:: \n", i);
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

void W_funcPara(double* x, double* x_new, int* danglings, int nodes, double d, double sum, double w){
  sum = 0.0;

    #pragma omp for reduction(+:sum)
    for( int i = 0; i <nodes; i++){
      if(danglings[i]){
        sum += x[i];
      }
    }

  #pragma omp single
  {
    w = (( (1 - d) + (d * sum) )/nodes);
  }
  #pragma omp for
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
    for( int j = 0; j < curr_amount; j++){
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
