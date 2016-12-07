//
// Created by tumbler on 06.12.16.
//
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <cuda_runtime_api.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "utils/argparse.h"
#include "als.h"
#include "host_utilities.h"

int DEVICEID = 0;
using namespace std;

off_t fileSize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

int* csrRowIndexHostPtr;
int* csrColIndexHostPtr;
float* csrValHostPtr;

int* cscRowIndexHostPtr;
int* cscColIndexHostPtr;
float* cscValHostPtr;

int* cooRowIndexHostPtr;

int* cooRowIndexTestHostPtr;
int* cooColIndexTestHostPtr;
float* cooValHostTestPtr;

float* thetaTHost;
float* XTHost;


void load_matrices(quokka_als_args* args, als_files* files, long nnz, long nnz_test) {

    cudaSetDevice(DEVICEID);
    cudacall(cudaMallocHost( (void** ) &csrRowIndexHostPtr, (args->m + 1) * sizeof(csrRowIndexHostPtr[0])) );
    cudacall(cudaMallocHost( (void** ) &csrColIndexHostPtr, nnz * sizeof(csrColIndexHostPtr[0])) );
    cudacall(cudaMallocHost( (void** ) &csrValHostPtr, nnz * sizeof(csrValHostPtr[0])) );
    cudacall(cudaMallocHost( (void** ) &cscValHostPtr, nnz * sizeof(cscValHostPtr[0])) );
    cudacall(cudaMallocHost( (void** ) &cscRowIndexHostPtr, nnz * sizeof(cscRowIndexHostPtr[0])) );
    cudacall(cudaMallocHost( (void** ) &cscColIndexHostPtr, (args->n+1) * sizeof(cscColIndexHostPtr[0])) );
    cudacall(cudaMallocHost( (void** ) &cooRowIndexHostPtr, nnz * sizeof(cooRowIndexHostPtr[0])) );

    //calculate X from thetaT first, need to initialize thetaT
    cudacall(cudaMallocHost( (void** ) &thetaTHost, args->n * args->factors * sizeof(thetaTHost[0])) );

    cudacall(cudaMallocHost( (void** ) &XTHost, args->m * args->factors * sizeof(XTHost[0])) );

    //initialize thetaT on host
    unsigned int seed = 0;
    srand (seed);
    for (int k = 0; k < args->n * args->factors; k++)
        thetaTHost[k] = (float) (0.1 * ((float) rand() / (float)RAND_MAX));
    //CG needs to initialize X as well
    for (int k = 0; k < args->m * args->factors; k++)
        XTHost[k] = 0;//0.1*((float) rand() / (float)RAND_MAX);;
    printf("*******start loading training and testing sets to host.\n");
    //testing set
    cooRowIndexTestHostPtr = (int *) malloc(
            nnz_test * sizeof(cooRowIndexTestHostPtr[0]));
    cooColIndexTestHostPtr = (int *) malloc(
            nnz_test * sizeof(cooColIndexTestHostPtr[0]));
    cooValHostTestPtr = (float *) malloc(nnz_test * sizeof(cooValHostTestPtr[0]));
    loadCooSparseMatrixBin(files->test_coo_data, files->test_coo_row, files->test_coo_col,
                           cooValHostTestPtr, cooRowIndexTestHostPtr, cooColIndexTestHostPtr, nnz_test);

    loadCSRSparseMatrixBin(files->train_csr_data, files->train_csr_indptr, files->train_csr_indices,
                           csrValHostPtr, csrRowIndexHostPtr, csrColIndexHostPtr, args->m, nnz);

    loadCSCSparseMatrixBin(files->train_csc_data, files->train_csc_indices, files->train_csc_indptr,
                            cscValHostPtr, cscRowIndexHostPtr, cscColIndexHostPtr, args->n, nnz);

    loadCooSparseMatrixRowPtrBin(files->train_coo_row, cooRowIndexHostPtr, nnz);

}


char* get_file_name(string prefix, const char *suffix) {
    string fn = prefix + suffix;
    char * result = (char*)malloc(fn.length() + 1);
    strcpy (result, fn.c_str());
    return result;
}


int main(int argc, char **argv) {
    quokka_als_args * args = parse_args(argc, argv);
    string train_prefix = string(args->train_matrix_prefix);

    als_files files;
    files.train_coo_row = get_file_name(train_prefix, TRAIN_COO_ROW);
    files.train_csr_data = get_file_name(train_prefix, TRAIN_CSR_DATA);
    files.train_csr_indices = get_file_name(train_prefix, TRAIN_CSR_INDICES);
    files.train_csr_indptr = get_file_name(train_prefix, TRAIN_CSR_INDPTR);
    files.train_csc_data = get_file_name(train_prefix, TRAIN_CSC_DATA);
    files.train_csc_indices = get_file_name(train_prefix, TRAIN_CSC_INDICES);
    files.train_csc_indptr = get_file_name(train_prefix, TRAIN_CSC_INDPTR);
    if (args->m == 0)
        args->m = fileSize(files.train_csr_indptr) / sizeof(int) - 1;
    if (args->n == 0)
        args->n = fileSize(files.train_csc_indptr) / sizeof(int) - 1;

    long nnz = fileSize(files.train_csr_data) / sizeof(float);
    if (nnz != fileSize(files.train_csc_data) / sizeof(float)) {
        cout << "CSC data and CSR data NNZ not match" << endl;
        return 0;
    }
    long nnz_test = 0;
    files.test_coo_data = NULL;
    files.test_coo_row = NULL;
    files.test_coo_col = NULL;
    if (args->test_matrix_prefix != NULL) {
        string test_prefix = string(args->test_matrix_prefix);
        files.test_coo_data = get_file_name(test_prefix, TEST_COO_DATA);
        files.test_coo_row = get_file_name(test_prefix, TEST_COO_ROW);
        files.test_coo_col = get_file_name(test_prefix, TEST_COO_COL);
        nnz_test = fileSize(files.test_coo_data) / sizeof(float);
    }

    cout << "DEBUG: " << fileSize(files.test_coo_data) << endl;
    cout << "M: " << args->m << endl;
    cout << "N: " << args->n << endl;
    cout << "Factors: " << args->factors << endl;
    cout << "Iterations: " << args->iterations << endl;
    cout << "Lambda: " << args->lambda << endl;
    cout << "X-batch: " << args->x_batch << endl;
    cout << "Theta-batch: " << args->t_batch << endl;

    cout << "Train data: " << endl;
    cout << "\t" << files.train_coo_row << endl;
    cout << "\t" << files.train_csr_data << endl;
    cout << "\t" << files.train_csr_indices << endl;
    cout << "\t" << files.train_csr_indptr << endl;
    cout << "\t" << files.train_csc_data << endl;
    cout << "\t" << files.train_csc_indices << endl;
    cout << "\t" << files.train_csc_indptr << endl;
    cout << " -- nnz: " << nnz << endl;
    if (nnz_test > 0) {
        cout << "Test data:" << endl;
        cout << "\t" << files.test_coo_data << endl;
        cout << "\t" << files.test_coo_row << endl;
        cout << "\t" << files.test_coo_col << endl;
        cout << " -- nnz_test: " << nnz_test << endl;
    }

    cout << "Output:" << endl;

    cout << args->x_output << endl;
    cout << args->t_output << endl;
    if (args->mean_output != NULL) {
        cout << "Mean normalize and output to: " << args->mean_output << endl;
    }

    cout << "Loading data to host..." << endl;

    load_matrices(args, &files, nnz, nnz_test);

    cout << "Randomize initial X, Theta" << endl;

    //initialize thetaT on host
    unsigned int seed = 0;
    srand (seed);
    for (int k = 0; k < args->n * args->factors; k++)
        thetaTHost[k] = (float) (0.1 * ((float) rand() / (float)RAND_MAX));
    //CG needs to initialize X as well
    for (int k = 0; k < args->m * args->factors; k++)
        XTHost[k] = (float) (0.1 * ((float) rand() / (float)RAND_MAX));;


    cout << "Do ALS..." << endl;

    doALS(csrRowIndexHostPtr, csrColIndexHostPtr, csrValHostPtr,
          cscRowIndexHostPtr, cscColIndexHostPtr, cscValHostPtr,
          cooRowIndexHostPtr, thetaTHost, XTHost,
          cooRowIndexTestHostPtr, cooColIndexTestHostPtr, cooValHostTestPtr,
          args->m, args->n, args->factors, nnz, nnz_test, args->lambda,
          args->iterations, args->x_batch, args->t_batch, DEVICEID);

    cout << "Saving X, Theta" << endl;
    //write out the model
	FILE * xfile = fopen(args->x_output, "wb");
	FILE * thetafile = fopen(args->t_output, "wb");
	fwrite(XTHost, sizeof(float), (size_t) (args->m * args->factors), xfile);
	fwrite(thetaTHost, sizeof(float), (size_t) (args->n * args->factors), thetafile);
	fclose(xfile);
	fclose(thetafile);

    cudaFreeHost(csrRowIndexHostPtr);
    cudaFreeHost(csrColIndexHostPtr);
    cudaFreeHost(csrValHostPtr);
    cudaFreeHost(cscValHostPtr);
    cudaFreeHost(cscRowIndexHostPtr);
    cudaFreeHost(cscColIndexHostPtr);
    cudaFreeHost(cooRowIndexHostPtr);
    cudaFreeHost(XTHost);
    cudaFreeHost(thetaTHost);
    cudacall(cudaDeviceReset());
    cout << "Done." << endl;
    return 0;
}