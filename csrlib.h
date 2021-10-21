#ifndef CSR_H
#define CSR_H

typedef struct CSR
{   uint64_t rows;
    uint64_t cols;
    __uint128_t nnz;
    //the 3 arrays
    float *values;
    uint64_t *col_indices;
    uint64_t *row_ptrs;
}CSR;
int initialize_CSR(uint64_t rows,
                   uint64_t cols,
                   __uint128_t nnz,
                   CSR *m);


double benchmark_mult(  void (*f)(const void *a, const void *b, void *result),
                        const CSR *a,
                        const CSR *b,
                        CSR *result,
                        int iterations
                    );
int file_to_csr(char *path,CSR *m);
int print_user_manual(char* path);
int valid_input(char *token, int type, char *path);
void csr_to_file(const CSR *m,char* pathResult);
void matr_mult(const CSR *a, const CSR *b, CSR *result);
int free_csr(CSR *m);

#endif

