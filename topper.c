#include "exam_header.h"
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
