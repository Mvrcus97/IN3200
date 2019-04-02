#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifndef EXAM_H_  /* Include guard */
#define EXAM_H_

#define CLOCKS_TO_MILLISEC(t) (t*1000)/CLOCKS_PER_SEC




//reader.c
void read_graph_from_file(int* nodes, int* edges, int** row_ptr, int** col_idx, double** val, int** danglings);
void count_to_and_from(char* string, FILE* fptr, int** from_ctr, int** to_ctr, int* nodes, int* edges);
void create_crs(char* string, FILE* fptr, int* to_ctr, int* from_ctr, int nodes, int edges, int** r, int** c, double** v, int** d);
void insertionSort(int *arr, int from, int to);
void bubbleSort(int *arr, int from, int to);
void insertionSort2(int arr1[], int arr2[], int n);
void skip_lines(char* string, FILE* fptr, int n);
int cmpfunc (const void * a, const void * b);

//ranker.c
void PageRank_iterations(int nodes, double d, double eps, int* row_ptr, int* col_idx, double* val, int* danglings, int itr, double** scores);
void W_funcSeq(double* x, double* x_tmp, int* danglings, int nodes, double d);
void W_funcPara(double* x, double* x_new, int* danglings, int nodes, double d);
void Ax(int nodes, double d, int* row_ptr, int* col_idx, double* val, double* x, double* x_tmp);
int early_stopping(int nodes, double* x, double* x_new, double eps);

//topper.c
void top_n_webpages(int nodes, int n, double *scores, double** top_n);
void top_k_sort(int nodes, int k, double *scores, int *page_idx);
void insertionSortDec(double* scores, int n, int* page_idx);




#endif // EXAM_H_
