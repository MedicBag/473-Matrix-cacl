#include <time.h>
#include <sys/time.h>
#include "threadpool.h"

typedef struct {
  int row;
  int col;
  int length;
} TaskArgs;

int **matrixA; // input
int **matrixB; // input
int **matrixC; // output

int** fillMatrix(int r, int c, int with0s) {
  int **mat = malloc(r * sizeof(int *));
  for (int i = 0; i < r; i++) {
    mat[i] = malloc(c * sizeof(int));
  }

  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      if(!with0s) mat[i][j] = rand() % 10;
      else mat[i][j] = 0;
    }
  }

  return mat;
}

int** fillMatrixFromFile(FILE *f, int r, int c) {
  int **mat = malloc(r * sizeof(int *));
  for (int i = 0; i < r; i++) {
    mat[i] = malloc(c * sizeof(int));
  }

  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      // Read the next integer from the file directly into the matrix
      if (fscanf(f, "%d", &mat[i][j]) != 1) {
          fprintf(stderr, "Error: Not enough data in file\n");
          exit(1);
      }
    }
  }

  return mat;
}

void printMatrix(int **matrix, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
}

void compute_partial_product(void *arg) {
  TaskArgs *args = (TaskArgs *)arg;

  int sum = 0;
  for (int k = 0; k < args->length; k++) {
    if (matrixA[args->row][k] != 0) {
      sum += matrixA[args->row][k] * matrixB[k][args->col];
    }
  }
  matrixC[args->row][args->col] = sum;
  free(args);
}

void save_result_to_file(const char *filename, int M, int N, int P, double time) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return;
    }
    fprintf(f, "Time took: %f seconds\n", time);
    fprintf(f, "matrix A (%d x %d):\n", M, N);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(f, "%d ", matrixA[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");

    fprintf(f, "matrix B (%d x %d):\n", N, P);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            fprintf(f, "%d ", matrixB[i][j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");

    fprintf(f, "matrix C (%dx%d):\n", M, P);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            fprintf(f, "%d ", matrixC[i][j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    printf("Results saved to output.txt");
}

int main(int argc, char *argv[]) {

  int M, N, N_check, P;

  if (argc != 3) {
    return 1;
  }
  printf("reading files\n");
  FILE *fileA = fopen(argv[1], "r");
  FILE *fileB = fopen(argv[2], "r");
  if (!fileA || !fileB) {
    perror("Error opening files");
    return 1;
  }

  fscanf(fileA, "%d %d", &M, &N);

  // Read N and P from the start of file B
  fscanf(fileB, "%d %d", &N_check, &P);

  if (N != N_check) {
    printf("Dimensions mismatch: %d != %d\n", N, N_check);
    exit(1);
  }

  total_tasks = M * P;

  pool_init(NUMBER_OF_THREADS);

  // Make 2 matrix where 1 is M x N and other is N x P
  matrixA = fillMatrixFromFile(fileA, M, N);
  matrixB = fillMatrixFromFile(fileB, N, P);
  matrixC = fillMatrix(M, P, 1); // this will hold the output

  struct timeval start, end;
  gettimeofday(&start, NULL);

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < P; j++) {
      TaskArgs *targs = malloc(sizeof(TaskArgs));
      targs->row = i;
      targs->col = j;
      targs->length = N;
      pool_submit(compute_partial_product, targs);
    }
  }

  sem_wait(&all_tasks_completed);

  gettimeofday(&end, NULL);
  double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
  printf("Wall time: %f seconds\n", time_taken);

  save_result_to_file("output.txt", M, N, P, time_taken);

  pool_shutdown();

  for (int i = 0; i < M; i++) {
    free(matrixA[i]);
    free(matrixC[i]);
  }

  for (int i = 0; i < N; i++) {
    free(matrixB[i]);
  }

  free(matrixA);
  free(matrixB);
  free(matrixC);

  return 0;
}
