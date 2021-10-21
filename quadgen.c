#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h> //to print uint64
#include <errno.h>
#include <time.h> //clock_gettime
#include <math.h>

int generate_n_quad_matr(char *path, int n)
{
    printf("please wait ...\n");
    FILE *output = fopen(path, "w");
    if (output == NULL)
    {
        fprintf(stderr, "error while opening file\n");
        return -1;
    }
    uint64_t rows = ((uint64_t)1 << n) - 1;
    //write first line: rows cols nnz (in a*b+c format)
    fprintf(output, "%" PRIu64 " %" PRIu64 " %" PRIu64 "*%d+0\n", rows, rows, rows, n);
    srand(time(NULL)); //different values for different files (new seed for number generator)
    for (uint64_t rowid = 0; rowid < rows; rowid++)
    {
        uint64_t col = 0;
        for (int nnz = 0; nnz < n; nnz++)
        {
            fprintf(output, "%f %" PRIu64 " %" PRIu64 "\n",
                    ((float)rand() / (float)(RAND_MAX)) * 109.467, rowid, col);
            col += (uint64_t)(rows / n);
        }
    }
    fclose(output);
    return 0;
};

int main(int argc, char **argv)
{

     if (argc != 3)
    {
        fprintf(stderr, "usage: gcc quadgen.c -o quadgen && ./quadgen n file.txt\nfor user manual use: make file && ./main -h\n");
        return -1;
    }
    int n = atoi(argv[1]);
    mkdir("./generated", S_IRWXU);
    size_t t = strlen(argv[2]);
    char *path;
    path = malloc(t + 12 + 1); // 19: dirpath + term, 1: '\0'
    if (!path)
    {
        fprintf(stderr, "error while allocationg memory\n");
        return -1;
    }
    strncpy(path, "./generated/", t + 12);
    strncat(path, argv[2], t + 1);

    if (generate_n_quad_matr(path, n))
        return -1;
    printf("%dquad matrix successfully generated at: \n%s\n", n, path);
    return 0;
}