
#include "exam_header.h"


void read_graph_from_file(int* n, int* e, int** r_ptr, int** c_idx, double ** v, int ** d){
  FILE *fptr;
  char string[256];
  int *to_ctr, *from_ctr, *row_ptr, *col_idx, *danglings;
  int nodes, edges;
  double *val;


  if ((fptr = fopen("web-NotreDame.txt", "r")) == NULL){
    printf("Error! opening file");
    exit(1);
   }

   //1)  - Count the number of FromNodeId and ToNodeId
   count_to_and_from(string, fptr, &to_ctr, &from_ctr, &nodes, &edges);
   fptr = fopen("web-NotreDame.txt", "r"); //reset file pointer

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
*   occurances of FromNodeId and ToNodeId there are for each node.
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
  printf("Nodes read: %d   Edges read:  %d\n",nodes, edges );
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

  printf("SELF LINKS: %d\n",self_link );


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

/* Function to sort one array and have the other array follow.
This is used to sort the FromNodeId, and have the ToNodeId follow same order.*/
void insertionSort2(int arr1[], int arr2[], int n){

    int i, key1, key2, j;
    for (i = 1; i < n; i++) {
        key1 = arr1[i];
        key2 = arr2[i];
        j = i - 1;

        /* Move elements of arr1[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
        while (j >= 0 && arr1[j] > key1) {
            arr1[j + 1] = arr1[j];
            arr2[j + 1] = arr2[j];
            j = j - 1;
        }
        arr1[j + 1] = key1;
        arr2[j + 1] = key2;
    }
}
