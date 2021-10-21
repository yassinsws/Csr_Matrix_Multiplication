#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>    // isdigit()
#include <inttypes.h> //PRIu64, strtoumax()
#include <time.h>     //clock_gettime
#include <malloc.h>
#include "csrlib.h"

int initialize_CSR(uint64_t rows,
                   uint64_t cols,
                   __uint128_t nnz,
                   CSR *m)
{
    m->rows = rows;
    m->cols = cols;
    m->nnz = nnz;
    //allocate memory for the CSR + memory allocation return value check
    m->values = calloc(nnz, sizeof(float));
    if (!m->values)
    {
        fprintf(stderr, "couldn't allocate memory while initializing CSR\n");
        return -1;
    }
    m->col_indices = calloc(nnz, sizeof(uint64_t));
    if (!m->col_indices)
    {
        fprintf(stderr, "couldn't allocate memory while initializing CSR\n");
        return -1;
    }
    m->row_ptrs = calloc(rows + 1, sizeof(uint64_t));
    if (!m->row_ptrs)
    {
        fprintf(stderr, "couldn't allocate memory while initializing CSR\n");
        return -1;
    }
    //if any memory allocation fails, free_csr will be used to avoid any memory leaks in the caller function
    return 0;
}

int free_csr(CSR *m) //avoids memory leaks
{
    free(m->row_ptrs);
    free(m->col_indices);
    free(m->values);
    return 0;
}

double benchmark_mult(void (*f)(const void *a, const void *b, void *result),
                      const CSR *a,
                      const CSR *b,
                      CSR *result,
                      int iterations)
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    //calling the passed in function
    for (int i = 0; i < iterations; i++)
    {
        (*f)(a, b, result);
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
    return time;
}

int valid_input(char *token, int type, char *path) //function to test if input is valid
{
    /*
    type 0: values
    type 1: nnz in normal format , rows , row index, cols, col index 
    type 2: nnz in a*b+c format
    */
    //counters
    int dots = 0; //only one '.' allowed (floats)
    int plus = 0; //only one '+' in the middle allowed (a*b+c format)
    int mul = 0;  //only one '*' in the middle allowed (a*b+c format)  
    if (type == 2 && strchr(token, '+') == NULL) //a*b+c format must have a, b and c, even if c = 0 => this line appends "+0" in case of a*b format
        strncat(token, "+0", 3);

    char *tok = token;
    if (*token != ' ' && strchr(token, ' ') != NULL)
    {
        fprintf(stderr, "<%s>:Invalid input in %s:\nEach line must contain three entries.\nAlso make sure there are no unecessary white spaces\n", tok, path);
        return -1;
    }
    if (*token == '-')
    {
        if (type == 0) //floats can be negative
            ++token;
        else
        {
            fprintf(stderr, "<%s> is invalid input in %s:\nnnz, rows and cols must be positive natural numbers including 0\n", tok, path);
            return -1;
        }
    }
    if (*token == '+') //first char can be '+' for all entries
        ++token;

    if (!*token)
    {
        fprintf(stderr, "<%s> is invalid input in %s:\n", token, path);
        return -1;
    }
    while (*token)
    {
        if (!isdigit(*token))
        {
            if (*token == '.')
            {
                if (type == 0 && dots == 0) //if first occurence of '.' in a float -> no problem
                {
                    token++;
                    dots++;
                    continue;
                }
                else
                    fprintf(stderr, "<%s> is invalid input in %s:\nCause: '.'\n", tok, path);
            }
            if (*token == '+')
            {
                if (type == 2 && plus == 0) //if first occurence of '+' in a (a*b+c) format) -> no problem
                {
                    token++;
                    plus++;
                    continue;
                }
                else
                    fprintf(stderr, "<%s> is invalid input in %s:\nCause: '+'\n", tok, path);
            }
            if (*token == '*')          //if first occurence of '*' in a (a*b+c) format) -> no problem
            {
                if (type == 2 && mul == 0)
                {
                    token++;
                    mul++;
                    continue;
                }
                else
                    fprintf(stderr, "<%s> is invalid input in %s:\nCause: '*'\n", tok, path);
            }
            //specifying some common mistakes for the user input
            if (*token == '\n')
                fprintf(stderr, "<%s> is invalid input in %s:\nunnecessary line break\n", tok, path);
            else if (*token == ' ')
                fprintf(stderr, "<%s> is invalid input in %s:\nunnecessary white space\n", tok, path);
            else if (*token == '+' || *token == '-')
                fprintf(stderr, "<%s> is invalid input in %s:\nmultiple signs\n", tok, path);
            else if (*token == ',')
                fprintf(stderr, "<%s> is invalid input in %s:\n',' are not accepted (use '.'for floats )\n", tok, path);
            fprintf(stderr, "To read user manual, please type ./main -h\n");
            return -1;
        }
        token++;
    }
    return 0;
}

__uint128_t readuint128(char *token) //parses __uint128_t from input in case of a*b+c format
{
    //if nnz is given in a*b (in case c=0), +0 will be appended before calling this function (guaranteeing a*b+c format)
    __uint128_t mynum;
    char *string128 = malloc(strlen(token) + 1);
    strcpy(string128, token);
    char *a;
    a = strtok(string128, "*");
    mynum = strtoumax(a, (char **)NULL, 10);
    a = strtok(NULL, "+");
    mynum = mynum * strtoumax(a, (char **)NULL, 10);
    a = strtok(NULL, " ");
    mynum = mynum + (strtoumax(a, (char **)NULL, 10));
    free(string128);
    return mynum;
}

//function to avoid redundant code, when errors happen in file_to_csr in case of invalid file.
int free_close_ret(void *filecontent, FILE *input)
{
    free(filecontent);
    fclose(input);
    return -1;
}

//function to create CSR from input file
int file_to_csr(
    char *path,
    CSR *m)
{
    /*******OPENING FILE + SAFETY CHECKS START HERE ***********/
    FILE *input = fopen(path, "r");
    if (!input)
    {
        fprintf(stderr, "error while opening %s\n", path);
        return -1;
    }
    struct stat ourStat;
    fstat(fileno(input), &ourStat);
    if (!S_ISREG(ourStat.st_mode))
    {
        fprintf(stderr, "%s is no regular file\n", path);
        fclose(input);
        return -1;
    }
    if (ourStat.st_size < 1)
    {
        fprintf(stderr, "%s size is 0\n", path);
        fclose(input);
        return -1;
    }
    char *filecontent = malloc(sizeof(char) * (ourStat.st_size + 1));
    if (!filecontent)
    {
        fprintf(stderr, "failed to allocate memory\n");
        return (free_close_ret(filecontent, input));
    }
    filecontent[ourStat.st_size] = '\0';

    int n = fread(filecontent, 1, ourStat.st_size, input);
    if (n != ourStat.st_size)
    {
        fprintf(stderr, "problem with file size\n");
        return (free_close_ret(filecontent, input));
    }
    /*******OPENING FILE + SAFETY CHECKS DONE ***********/
    //filecontent now contains all of our file content
    //creating copy of filecontent, because strtok may be used in readuint128, will not be needed after parsing line0
    char *filecontentcopy = malloc(strlen(filecontent) + 1);
    if (!filecontentcopy)
    {
        fprintf(stderr, "failed to allocate memory\n");
        return -1;
    }
    strcpy(filecontentcopy, filecontent);
    //parsing from line0 starts here:
    char *token;
    token = strtok(filecontentcopy, " "); //extract first token in first token (rows)
    if (valid_input(token, 1, path))
    {
        free(filecontentcopy);
        return (free_close_ret(filecontent, input));
    }
    uint64_t rows = strtoumax(token, (char **)NULL, 10);
    token = strtok(NULL, " "); //extract next token (columns)
    if (valid_input(token, 1, path))
    {
        free(filecontentcopy);
        return (free_close_ret(filecontent, input));
    }
    uint64_t cols = strtoumax(token, (char **)NULL, 10);
    __uint128_t nnz = 0;

    token = strtok(NULL, "\n"); //extract last token (nnz)

    if ((strchr(token, '*') != NULL))
    { //nzz in a*b +c format
        if (valid_input(token, 2, path))
        {
            free(filecontentcopy);
            return (free_close_ret(filecontent, input));
        }
        nnz = readuint128(token);
    }
    else
    { //nnz in normal format
        if (valid_input(token, 1, path))
        {
            free(filecontentcopy);
            return (free_close_ret(filecontent, input));
        }
        nnz = strtoumax(token, (char **)NULL, 10);
    }
    free(filecontentcopy);
    //parsing line0 ends here //

    //initialize the CSR
    if (initialize_CSR(rows, cols, nnz, m))
    {
        fprintf(stderr, "error while initializing CSR from %s\n", path);
        free_csr(m);
        return (free_close_ret(filecontent, input));
    };

    //store rest of the file
    __uint128_t nnz_index = 0; //current value index
    uint64_t rowid = 0;        //current rowptrs index
    uint64_t row = 0;          //row index from input
    uint64_t col = 0;          //col index from input
    //first rowptrs entry is always 0
    m->row_ptrs[rowid] = 0;
    token = strtok(filecontent, "\n");
    token = strtok(NULL, " "); //prepare next token (first value)
    if (token == NULL)
    {
        if (nnz == 0)
        { //nullmatrix
            free_close_ret(filecontent, input);
            return 0;
        }
        else
        {
            fprintf(stderr, "nnz != 0 but no given values \n");
            return (free_close_ret(filecontent, input));
        }
    }
    else if (nnz == 0)
    {
        fprintf(stderr, "nnz == 0 but values are given \n");
        return (free_close_ret(filecontent, input));
    }
    if (valid_input(token, 0, path))
        return (free_close_ret(filecontent, input));
    while (token != NULL) //go over lines from the file, extracting 3 values each time
    {
        //expected number of lines must correspond to nnz
        if (nnz_index >= m->nnz)
        {
            fprintf(stderr, "Incorrect input format in %s:\nAfter first line, number of given lines exceeds the given nnz number. \nPlease correct nnz or check missing/extra lines\n", path);
            return (free_close_ret(filecontent, input));
        }
        m->values[nnz_index] = strtof(token, NULL);
        token = strtok(NULL, " "); //prepare next token (row)
        if (valid_input(token, 1, path))
            return (free_close_ret(filecontent, input));

        row = strtof(token, NULL);
        //check correct order of row indices
        if (row < rowid)
        {
            fprintf(stderr, "Invalid input in %s:\nPlease check order of rows\n", path);
            return (free_close_ret(filecontent, input));
        }
        //check if row index is out of bound
        if (row >= m->rows)
        {
            fprintf(stderr, "Invalid input in %s:\nRow index out of bound\n", path);
            return (free_close_ret(filecontent, input));
        }
        token = strtok(NULL, "\n"); //prepare last token (col index)
        if (valid_input(token, 1, path))
            return (free_close_ret(filecontent, input));

        col = strtof(token, NULL);
        //check if col index is out of bound
        if (col >= m->cols)
        {
            fprintf(stderr, "Invalid input in %s:\nColumn index out of bound\n", path);
            return (free_close_ret(filecontent, input));
        }
        m->col_indices[nnz_index] = col;
        //check correct order of column indices
        if (row == rowid && nnz_index > 0)
        {
            if (m->col_indices[nnz_index] <= m->col_indices[nnz_index - 1])
            {
                fprintf(stderr, "Invalid input in %s:\nPlease check order of col_indices\n", path);
                return (free_close_ret(filecontent, input));
            }
        }
        token = strtok(NULL, " "); //prepare next token (next value)
        if (token != NULL && valid_input(token, 0, path))
            return (free_close_ret(filecontent, input));

        while (rowid != row) //update rowptr in case we are in a new row
        {
            rowid++;
            m->row_ptrs[rowid] = nnz_index;
        }
        nnz_index++;
    }
    //expected number of lines must correspond to nnz
    if (nnz_index != m->nnz)
    {
        fprintf(stderr, "Incorrect input format in %s:\nAfter first line, number of given lines is less the given nnz number. \nPlease correct nnz or check missing/extra lines\n", path);
        return (free_close_ret(filecontent, input));
    }
    //fill last entries of rowptrs
    while (rowid != m->rows)
    {
        rowid++;
        m->row_ptrs[rowid] = nnz_index;
    }
    //file operations done
    free_close_ret(filecontent, input);
    return 0;
}

int print_user_manual(char *path)
{
    FILE *user_man = fopen(path, "r");
    char c;
    while ((c = fgetc(user_man)) != EOF)
        putchar(c);
    fclose(user_man);
    return 0;
}

void csr_to_file(const CSR *m, char *pathResult)
{
    FILE *output = fopen(pathResult, "w");
    if (!output)
    {
        fprintf(stderr, "error while opening file to save result\n");
        return;
    }
    const uint64_t MAX_UINT64 = 18446744073709551615U;
    if (m->nnz <= MAX_UINT64)
        fprintf(output, "%" PRIu64 " %" PRIu64 " %" PRIu64 "\n", m->rows, m->cols, (uint64_t)m->nnz);
    else
    { //m->nnz > MAX_UINT64 => a*b+c format
        __uint128_t b = m->nnz / (__uint128_t)(MAX_UINT64);
        __uint128_t c = m->nnz - b * (__uint128_t)(MAX_UINT64);
        fprintf(output, "%" PRIu64 " %" PRIu64 " 18446744073709551615*%" PRIu64 "+%" PRIu64 "\n",
                m->rows, m->cols, (uint64_t)b, (uint64_t)c);
    }
    for (uint64_t rowid = 0; rowid < m->rows; rowid++)
    {
        for (uint64_t valid = m->row_ptrs[rowid]; valid < m->row_ptrs[rowid + 1]; valid++)
            fprintf(output, "%f %" PRIu64 " %" PRIu64 "\n", m->values[valid], rowid, m->col_indices[valid]);
    }
    fclose(output);
    printf("Result successfully saved at: \n%s\n", pathResult);
}
