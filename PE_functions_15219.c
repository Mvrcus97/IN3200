
#include "exam_header.h"

void print_line(){ printf("------------------------------------------------------\n");}


// -------------------------------------------------PART 1 - READING THE FILE ----------------------------

void read_graph_from_file(char filename[], int* n, int* e, int** r_ptr, int** c_idx, double ** v, int ** d){
  FILE *fptr;
  char string[256];
  int *to_ctr, *from_ctr, *row_ptr, *col_idx, *danglings;
  int nodes, edges;
  double *val;


  if ((fptr = fopen(filename, "r")) == NULL){
    printf("Error opening file. [%s]\n", filename);
    exit(1);
   }

   //1)  - Count the number of FromNodeId and ToNodeId
   count_to_and_from(string, fptr, &to_ctr, &from_ctr, &nodes, &edges);
   fptr = fopen(filename, "r"); //reset file pointer

   //2) -  Create the CRS
   create_crs(string, fptr, to_ctr, from_ctr, nodes, edges, &row_ptr, &col_idx, &val, &danglings);

   //Return back to main
   *n = nodes;
   *e = edges;
   *r_ptr = row_ptr;
   *c_idx = col_idx;
   *v = val;
   *d  = danglings;
   fclose(fptr);
}//end read_graph_from_file


// Read n- amount of lines without doing anything with them.
void skip_lines(char* string, FILE* fptr, int n){
  for( int i = 0; i < n; i++){
    fgets(string, 100, fptr);
  }
}

/*  This function creates 2 arrays used to count how many
*   occurrences of FromNodeId and ToNodeId there are for each node.
*   Finally updates how many total nodes and edges there are, as well
*   as returning the count arrays.
*/
void count_to_and_from(char* string, FILE* fptr, int** to_ctr, int** from_ctr, int* n, int* e){
  skip_lines(string, fptr, 2); //skip first 2 lines.
  fgets(string,100,fptr);
  int nodes, edges;
  sscanf(string, "# Nodes: %d Edges: %d", &nodes, &edges);
  int fromID, toID;
  int *from_counter = malloc(sizeof(int)*nodes);
  int *to_counter = malloc(sizeof(int)*nodes);
  print_line();
  printf(" Web Graph contains [%d] Nodes and [%d] Edges\n",nodes, edges );
  skip_lines(string, fptr, 1); //skip next line: "# FromNodeId ToNodeId".

  //Read all values from file
  while(fgets(string,100,fptr)){
    sscanf(string, "%d %d", &fromID, &toID);
    if(fromID == toID) continue;
    to_counter[toID] ++;
    from_counter[fromID] ++;
  }// end while


  *to_ctr = to_counter;
  *from_ctr = from_counter;
  *n = nodes;
  *e = edges;

}//end count_to_and_from


/*  create_crs is used to create the following arrays of values:
* Row_ptr, Col_idx, Value.  Togethere these can be used as a Compressed Row Storage
* of the Sparse Matrix containing the webgraph information.
*
*/
void create_crs(char* string, FILE* fptr, int* to_ctr, int* from_ctr, int nodes, int edges, int **r, int** c, double ** v, int** d){
  int *col_idx = malloc(sizeof(int)*edges);
  int *row_ptr = malloc( (sizeof(int)*nodes)+1);
  double *val = malloc(sizeof(double)*edges);
  //Arrays used in CRS.

  //Initialize arrays
  for(int i = 0; i < edges; i++){
    if(i<nodes+1) row_ptr[i] = 0;
    col_idx[i] = 0;
    val[i] = 0.0;
  }// init done

  int curr_row_idx = 1;
  int fromID, toID;
  int i;

  // Place correct values of the row_ptr.
  for(i = 1; i<=nodes+1; i++){
    row_ptr[curr_row_idx] = row_ptr[curr_row_idx-1] + to_ctr[i-1];
    curr_row_idx ++;
  }
  ;
  //Read file into two arrays.
  int *danglings = malloc(sizeof(int)*nodes);
  int *fromArr = malloc(edges * sizeof(*fromArr));
  int *toArr   = malloc(edges * sizeof(*toArr));

  //Begin by setting all webpages to dangling webpages.
  for(i = 0; i<nodes; i++){
    danglings[i] = 1;
  }

  int curr_idx;
  int self_link = 0;
  i = 0;
  skip_lines(string, fptr, 4); //skip the first 4 lines in txt file
  while(fgets(string,100,fptr)){
    //Read all values from file, store in correct array.
    sscanf(string, "%d %d", &fromID, &toID);
    fromArr[i] = fromID;
    toArr[i]   =   toID;
    if(fromID == toID){
      self_link ++;
      //printf("SKIPPING\n" );
      continue;
    }
    danglings[fromID] = 0;//not a dangling webpage.
    //add to col_idx.
    curr_idx = row_ptr[toID];
    col_idx[curr_idx + to_ctr[toID] - 1] = fromID;
    to_ctr[toID] --;
    i++;
  }//end while

  printf("           [%d] Self Linkage Found.\n",self_link );
  print_line();

  // col_idx for each row is now correct, but we need them in ascending order.
  for(int i = 0; i < nodes+1; i++){
    if(row_ptr[i+1] - row_ptr[i] > 1){
      qsort(&col_idx[row_ptr[i]], row_ptr[i+1]-row_ptr[i], sizeof(int), cmpfunc);
    }
  }



  // Create the values for the val array.
  int tmp_num = 0;
  int tmp_divident = 0;
  int val_idx = 0;
  for(i = 0; i < edges-self_link; i++){
    tmp_num = col_idx[i];
    tmp_divident = from_ctr[tmp_num];
    val[val_idx] = 1.0/tmp_divident;
    val_idx++;
  }

  //Update parameters and free the count-arrays.
  *r = row_ptr;
  *c = col_idx;
  *v = val;
  *d = danglings;

  free(to_ctr);
  free(from_ctr);

}//end create_crs


int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

// ------------------------ END PART 1-------------------------------







//------------PART 2 - GIVING THE PAGES A RANKING -------------------------


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

  printf("AVAIBLE THREADS FUNC: %d\n",omp_get_num_threads());
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
      if(max < eps) stop = 1;
    }

    if(stop)break;

  }//End for iteration
  #pragma omp single
  {
    done = i; //How many iterations did it take?
  }
  }//end Parallel

  printf("   Converged after [%d] iterations. \n", done);
  printf("   Time W: [%lfs]   Time Ax: [%lfs]\n",totW, totA );
  print_line();

  //Finally update output to the correct webpage ranks.
  *scores = x;
}//end PageRank_iterations



/*
*     THE FUNCTIONS BELOW ARE SEQUENTIAL SOLUTIONS OF THE DIFFERENT STEPS DONE ABOVE.
*     THESE CAN BE USED TO GET A BETTER UNDERSTANDING OF HOW THE ALGOIRTHM ABOVE WORKS.
*
*/

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

/* This function is used to debug the CRS. Each columns should add up to either 0 or 1. */
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

//------------------------------END PART 2 - RANKING ------------------------------------------------





// ----------------PART 3 - FINDING THE TOP N PAGES ---------------------------------

/*  Find the top n pages, and print their webpage number aswell as their score.
* This is done by implementing a top-k-sort, while keeping track of where the original
* indicies are located.
*/
void top_n_webpages(int nodes, int n, double* scores, double** top_n){
  double *top_n_scores = malloc(sizeof(double*)*n);
  int *page_idx = malloc(sizeof(int*)*nodes);
  top_k_sort(nodes, n, scores, page_idx);

  printf("                Top %d Pages\n", n );
  for(int i = 0; i < n; i++){
    printf("%d) -   WebPage[%d]   Score: [%f]  \n",i+1, page_idx[i], scores[i] );
  }
  print_line();

  for(int i = 0; i <n; i++){
    top_n_scores[i] = scores[i];
  }
*top_n = top_n_scores;
}//end top_n_webpages


 /* Find the top k elements. This is done by first assuming the first k elements are the top k elements, and sorting them.
 * Thereafter compare each elemnt with the worst top k. if the current element is larger than the worst K, swap them and
 * re-sort the top k elements.
 * Update paramter with an array of all webpages, where the top k elements are located first.
 */
void top_k_sort(int nodes, int k, double *scores, int *page_idx){
  int i;
  double tmp;

  for(int i = 0; i<nodes; i++){
    page_idx[i] = i;
  }

  insertionSortDec(scores, k-1, page_idx); //Sort top k elements.


  for(i = k; i < nodes; i++){
    if(scores[i] > scores[k]){
      //Swap positions i and k, and sort the top k elements again.
      tmp = scores[i];
      scores[i] = scores[k];
      page_idx[i] = k;
      scores[k] = tmp;
      page_idx[k] = i;
      insertionSortDec(scores, k, page_idx); //Sort top k elements.
    }
  }
}//end top_k_sort


/* Descending insertion sort, from 0 to n.
* Update scores to have the top n sorted. Also keep track of the original indicies.
*/
void insertionSortDec(double* scores, int n, int* page_idx){
  double tmp;
  int i, tmp_i;

  for(int k = 0; k<n; k++){
      tmp = scores[k+1];
      tmp_i = page_idx[k+1];
      i=k;
      while(i>=0 && scores[i]<tmp){
          scores[i+1] = scores[i];
          page_idx[i+1] = page_idx[i];
          i--;
      }
      scores[i+1] = tmp;
      page_idx[i+1] = tmp_i;
    } // end for k.
}//end insertionSortDec

// ---------------------------END PART 3 - FINDING THE TOP N PAGES. --------------------
