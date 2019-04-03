#include "exam_header.h"


int main(int argc, char **args) {
  if (argc != 6) {
      print_line();
      printf("Usage: ./run 'file_name' 'damping_factor' 'epsilon' 'top_n_webpages' 'threads'\n");
      print_line();
      printf("Example run with all avaible threads:\n" );
      printf("./run web-NotreDame.txt 0.85 0.0000000000001 10 0\n" );
      print_line();
      return 0;
  }
  char filename[32];
  strncpy(filename, args[1], 32);
  double damping = atof(args[2]);
  double epsilon = atof(args[3]);
  int n = atoi(args[4]);
  int threads = atoi(args[5]);

  if(n <0){
    printf("You want to show top %d pages? Come on man..\n",n);
    return 0;
  }

  if( damping >= 1 || damping <= 0){
    printf("Please use a damping factor between 0 and 1. \nDamping found: [%lf]\n", damping);
    return 0;
  }

  if( threads == 0){
    threads = omp_get_max_threads();
  }
  omp_set_dynamic(0);  //Force openMP to use given number of threads
  omp_set_num_threads(threads); //Set threads

  int *row_ptr, *col_idx, *danglings;
  double *val, *scores, *top_n_scores;
  int nodes, edges;
  double start, end, tot;
  double start_tot, end_tot, tot_tot;

  start = omp_get_wtime(); // Returns time in seconds.
  start_tot = omp_get_wtime();
  read_graph_from_file(filename, &nodes, &edges, &row_ptr, &col_idx, &val, &danglings);
  end = omp_get_wtime();
  tot = end - start;
  printf("    Time Result - Read Graph: [%fs]\n",tot);
  print_line();


  start = omp_get_wtime(); // Returns time in seconds.
  PageRank_iterations(nodes, edges, damping, epsilon, row_ptr, col_idx, val, danglings, 1000, &scores);//max 1000 iterations.
  end = omp_get_wtime();
  tot = end - start;
  printf("    Time Result - Page Rank: [%fs]\n",tot);
  print_line();

  if(n > nodes){
      printf("Can't print out top %d web pages, there only exist %d!\n",(int)n, nodes );
      n = nodes;
  }
  start = omp_get_wtime(); // Returns time in seconds.
  top_n_webpages(nodes, n, scores, &top_n_scores);
  end = omp_get_wtime();
  tot = end - start;
  printf("    Time Result - Top n WebPages: [%fs]\n",tot);
  print_line();

  end_tot = omp_get_wtime();
  tot_tot = end_tot - start_tot;
  printf("          TOTAL RUNTIME: %fs \n",tot_tot);
  print_line();

  free(row_ptr);
  free(col_idx);
  free(danglings);
  free(val);
  free(scores);
  free(top_n_scores);

  return 0;
}
