#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MROWS 100
#define CORE_LIMIT 12

int main(int argc, char** argv)
{
    const int count = 10000000;     ///< Number of array elements
    const int random_seed = 31337; ///< RNG seed

    int** matrix = 0;              ///< The array we need to find the max in
    int  max = -1;                ///< The maximal element
    double start, end;

    srand(random_seed);

    /* Generate the random MATRIX */
    matrix = (int**)calloc(MROWS, sizeof(int*));
    for (int i = 0; i < MROWS; i++) {
        matrix[i] = (int*)calloc(count, sizeof(int));
    }
    for (int i = 0; i < MROWS; i++) {
        for (int elem = 0; elem < count; elem++) {
            matrix[i][elem] = rand();
        }
    }
    char filename[50];
    sprintf(filename, "trace.txt");
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Can't open file\n");
        exit(1);
    }
    for (int thread = 1; thread <= CORE_LIMIT; thread++) {
        for (int row = 0; row < MROWS; row++) {
            max = -1;
            start = omp_get_wtime();
            #pragma omp parallel num_threads(thread) shared(matrix, count, row) reduction(max: max) default(none)
            {
                #pragma omp for
                for(int i=0; i<count; i++)
                {
                    if(matrix[row][i] > max) { 
                        max = matrix[row][i]; 
                    };
                }
            }
            end = omp_get_wtime();
            printf("CURR_THREAD = %d\n", thread);
            fprintf(fp, "%f;", end-start);
            
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("Exit...\n");
    return(0);
}