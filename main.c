#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include "csrlib.h"

extern void matr_mult_csr(const void *a, const void *b, void *result);
extern void matr_mult_csr_SIMD(const void *a, const void *b, void *result);
extern void matr_mult_csr2(const void *a, const void *b, void *result);

int call_mult(char *pathA, char *pathB, char *pathResult, int impl, int bench)
{
    //in case of an error while parsing csr from file, file_to_csr will output error messages and free the allocated memory for the matrices.
    CSR a;
    if (file_to_csr(pathA, &a))
        return -1;
    CSR b;
    if (file_to_csr(pathB, &b))
        return -1;
    //check compatibility of the dimensions, otherwise multiplication is undefined
    if (a.cols != b.rows)
    {
        fprintf(stderr, "Multiplication is not defined for Matrices with sizes %lux%lu and %lux%lu \n", a.rows, a.cols, b.rows, b.cols);
        free_csr(&a);
        free_csr(&b);
        return -1;
    }
    CSR result;
    //result_nnz can never exceed a.nnz*b*nnz or a.rows*b.cols, therefore we assign the minimum of the two until csr_result is determined
    __uint128_t result_nnz = (a.nnz * b.nnz) < (a.rows * b.cols) ? (a.nnz * b.nnz) : a.rows * b.cols;

    if (initialize_CSR(a.rows, b.cols, result_nnz, &result))
    {
        fprintf(stderr, "error while initializing result CSR \n");
        free_csr(&result);
        free_csr(&a);
        free_csr(&b);
        return -1;
    }

    if (a.nnz == 0 || b.nnz == 0)
    /*in case of a nullmatrix in one of the operands, time is saved by directly saving the result_csr
     at the initialized state (nullmatrix) instead of calling matr_mult_csr*/
    {
        csr_to_file(&result, pathResult);
        free_csr(&a);
        free_csr(&b);
        free_csr(&result);
        return 0;
    }

    double time = 0;    //time will be returned after measuring performance
    int iterations = 1; //number of iterations that will be used for performace measurement

    switch (impl) //impl determines what implementation to run
    {
    case 0: //asm1 SIMD (./main [-t] [-q])
        time = benchmark_mult(matr_mult_csr_SIMD, &a, &b, &result, iterations);
        break;
    case 1: //asm1 SISD (./main -u [-t] [-q])
        time = benchmark_mult(matr_mult_csr, &a, &b, &result, iterations);
        break;
    case 2: //asm2 SISD (./main -2 [-t] [-q])
    case 3: //asm2 SISD (./main -2 -u [-t] [-q])
        time = benchmark_mult(matr_mult_csr2, &a, &b, &result, iterations);
        break;
    default:
        fprintf(stderr, "Problem with implementation choice\nuse ./main -h for user manual\n");
    }
    if (time && bench)
    {
        printf("time in seconds after %d iteration(s):\n%0.10f\n", iterations, time);
        printf("Average time for 1 execution in microseconds :\n %f\n\n", (time / iterations) * 1000000);
    }
    csr_to_file(&result, pathResult);
    free_csr(&a);
    free_csr(&b);
    free_csr(&result);
    return 0;
}

int create_path(char *path, char *dirpath, char *d_name, char *term /*/inputA.txt*/) //avoids redundant code at run_tests
{
    size_t n = strlen(d_name);
    strncpy(path, dirpath, n + 19);
    strncat(path, d_name, n + 1);
    strncat(path, term, 12);
    return 0;
}

int run_tests(char *dirpath, int impl, int bench)
{
    DIR *tests_dir;
    struct dirent *test_sub;
    tests_dir = opendir(dirpath);
    while ((test_sub = readdir(tests_dir)) != NULL)
    {
        if (test_sub && test_sub->d_type == 4 && *test_sub->d_name != '.')
        {
            size_t n = strlen(test_sub->d_name);
            char *inputA;
            inputA = malloc(n + 19 + 1); // 19: dirpath + term, 1: '\0'
            if (!inputA)
            {
                fprintf(stderr, "error while allocationg memory\n");
                return -1;
            }
            char *inputB;
            inputB = malloc(n + 20);
            if (!inputB)
            {
                fprintf(stderr, "error while allocationg memory\n");
                return -1;
            }
            char *resultPath;
            resultPath = malloc(n + 20);
            if (!resultPath)
            {
                fprintf(stderr, "error while allocationg memory\n");
                return -1;
            }

            create_path(inputA, dirpath, test_sub->d_name, "/inputA.txt");
            create_path(inputB, dirpath, test_sub->d_name, "/inputB.txt");
            create_path(resultPath, dirpath, test_sub->d_name, "/result.txt");
            call_mult(inputA, inputB, resultPath, impl, bench);

            free(inputA);
            free(inputB);
            free(resultPath);
            printf("\n");
        }
    }
    closedir(tests_dir);
    return 0;
}

//main program
int main(int argc, char **argv)
{
    int test_flag = 0;
    int help_flag = 0;
    int benchmark_flag = 0;
    int implementation = 0;
    int option = 0;
    int quad_test = 0;

    while ((option = getopt(argc, argv, "thubaq2")) != -1)
    {
        switch (option)
        {
        case 't':
            test_flag = 1;
            break;
        case 'h':
            help_flag = 1;
            break;
        case 'q':
            quad_test = 1;
            break;
        case 'b':
            benchmark_flag = 1;
            break;
        case 'u':
            implementation += 1;
            break;
        case '2':
            implementation += 2;
            break;
        default:
            fprintf(stderr, "use %s -h for user manual\n", argv[0]);
            return 1;
        }
    }
    if (help_flag)
    {
        if (print_user_manual("README_Benutzerhandbuch.txt"))
            return -1;
        return 0;
    }
    if (test_flag)
    {
        if (quad_test)
        {
            if (run_tests("./nquad/", implementation, benchmark_flag))
            {
                fprintf(stderr, "error while running tests in ./nquad/\n");
                return -1;
            }
        }
        else
        {
            if (run_tests("./tests/", implementation, benchmark_flag))
            {
                fprintf(stderr, "error while running tests in ./tests/\n");
                return -1;
            }
        }
    }
    else // default mode
    {
        char *pathA = "./inputA.txt";
        char *pathB = "./inputB.txt";
        char *pathResult = "./result.txt";
        call_mult(pathA, pathB, pathResult, implementation, benchmark_flag);
    }
    return 0;
}
