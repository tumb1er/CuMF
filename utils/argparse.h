//
// Created by tumbler on 06.12.16.
//

#ifndef CUMF_ARGPARSE_H
#define CUMF_ARGPARSE_H

#define TEST_COO_DATA ".test.coo.data.bin"
#define TEST_COO_ROW ".test.coo.row.bin"
#define TEST_COO_COL ".test.coo.col.bin"
#define TRAIN_COO_ROW ".train.coo.row.bin"
#define TRAIN_CSR_DATA ".train.csr.data.bin"
#define TRAIN_CSR_INDICES ".train.csr.indices.bin"
#define TRAIN_CSR_INDPTR ".train.csr.indptr.bin"
#define TRAIN_CSC_DATA ".train.csc.data.bin"
#define TRAIN_CSC_INDPTR ".train.csc.indptr.bin"
#define TRAIN_CSC_INDICES ".train.csc.indices.bin"


typedef struct _quokka_als_args {
    unsigned int m; // number of rows
    unsigned int n; // number of columns
    unsigned int factors; // factors count
    unsigned int iterations; // number of iterations

    float lambda; // hyper-parameter
    unsigned int x_batch;  // X batch size
    unsigned int t_batch;  // Theta batch size

    // Test data: sparse matrix in COO format (x + y + value)
    char* test_matrix_prefix; // common prefix for files .test.data.bin, .test.row.bin, .test.col.bin

    // Train data in COO, CSR and CSC formats with prefixes:
    // .train.csr.data.bin, train.csr.indptr.bin, train.csr.indices.bin
    // .train.csc.data.bin, train.csc.indices.bin, train.csc.indptr.bin
    // .train.coo.data.bin, .train.coo.row.bin, .train.coo.col.bin
    char* train_matrix_prefix; // common prefix for files
    char* x_output; // X dense matrix filename;
    char* t_output; // Theta dense matrix filename;
    char* mean_output; // Per-video mean shift output filename;

} quokka_als_args;

typedef struct _als_files {
    const char * train_coo_row;

    const char * train_csr_data;
    const char * train_csr_indices;
    const char * train_csr_indptr;

    const char * train_csc_data;
    const char * train_csc_indices;
    const char * train_csc_indptr;
    const char * test_coo_data;
    const char * test_coo_row;
    const char * test_coo_col;

} als_files;

/// Разбирает аргументы командной строки для программы
/// \param argc число аргументов
/// \param argv массив указателей на входные параметры программы
/// \return указатель на структуру с разобранными параметрами
quokka_als_args* parse_args(int argc, char **argv);

#endif //CUMF_ARGPARSE_H
