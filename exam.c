#include "exam_header.h"



int main(int number, char **args) {
  int *row_ptr, *col_idx, *danglings;
  double *val, *scores, *top_n_scores;
  int nodes, edges;

  read_graph_from_file(&nodes, &edges, &row_ptr, &col_idx, &val, &danglings);
  printf("Read Graph Done.\n" );



  double start = omp_get_wtime(); // Returns time in seconds.
// ... Some smart calculation here.

  PageRank_iterations(nodes, edges, 0.85, 0.000000000001, row_ptr, col_idx, val, danglings, 1000, &scores);
  double end = omp_get_wtime();
  double tot = end - start;

  printf("Time PageRank: %fs\n",tot);
  printf("PageRank Done.\n" );
  top_n_webpages(nodes, 10, scores, &top_n_scores);


free(row_ptr);
free(col_idx);
free(danglings);
free(val);
free(scores);
free(top_n_scores);



  return 0;
}
