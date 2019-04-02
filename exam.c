#include "exam_header.h"



int main(int number, char **args) {
  int *row_ptr, *col_idx, *danglings;
  double *val, *scores, *top_n_scores;
  int nodes, edges;

  read_graph_from_file(&nodes, &edges, &row_ptr, &col_idx, &val, &danglings);
  printf("Read Graph Done.\n" );

  for(int i = 0; i < edges; i++){

  }

  clock_t start, total;
  start = clock();
  PageRank_iterations(nodes, 0.85, 0.000001, row_ptr, col_idx, val, danglings, 200, &scores);
  total = clock() - start;
  printf("Clock cycles PARA method: %ld\n",total);
  printf("PageRank Done.\n" );
  top_n_webpages(nodes, 10, scores, &top_n_scores);



  return 0;
}
