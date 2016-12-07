#include <stdio.h>

#include <getopt.h>
#include <stdlib.h>
#include "argparse.h"


void print_usage(char *prog) {
    printf("Usage: %s [options] <train_data_prefix> <X_output> <Theta_output>\n", prog);
    printf("Options:\n");

    printf("--test <prefix>\ttest data prefix\n");
    printf("--factors <int>\tnumber of factors (multiple of 10, default 100)\n");
    printf("--xbatch <int>\tX batch count\n");
    printf("--tbatch <int>\tTheta batch count\n");
    printf("--M <int>\trows count\n");
    printf("--N <int>\tcolumns count\n");
    printf("--iterations <int>\tNumber of iterations\n");
    printf("--normalize <filename>\tnormalize input data and save coefficients to <filename>\n");
    exit(1);
}



quokka_als_args* parse_args(int argc, char **argv) {
    char* pointers[ARGS_COUNT];
    int c;
    int option_index = 0;
    int value = 0;
    quokka_als_args* args = (quokka_als_args*)malloc(sizeof(quokka_als_args));
    args->mean_output = NULL;
    args->test_matrix_prefix = NULL;
    args->factors = 100;
    args->iterations = 10;
    args->lambda = 0.3;
    args->m = 0;
    args->n = 0;
    if (argc <= ARGS_COUNT) {
        print_usage(argv[0]);
        return NULL;
    }

    while (1) {
        static struct option long_options[] =
        {
                {"factors", required_argument, 0,            'f'},
                {"normalize", required_argument, 0,            'n'},
                {"test", required_argument, 0,            'T'},
                {"lambda", required_argument, 0,            'l'},
                {"xbatch", required_argument, 0,            'x'},
                {"tbatch", required_argument, 0,            't'},
                {"M", required_argument, 0,            'M'},
                {"N", required_argument, 0,            'N'},
                {"iterations", required_argument, 0,            'i'},
                {0, 0,                         0,            0}
        };
        c = getopt_long(argc, argv, "f:l:n:x:t:i:M:N:", long_options, &option_index);
        if (c == -1) break;
        switch(c) {
            case 'f':
                value = atoi(optarg);
                if (value <= 0 || value % 10) {
                    print_usage(argv[0]);
                    return NULL;
                }
                args->factors = (unsigned int) value;
                break;
            case 'l':
                args->lambda = (float) atof(optarg);
                break;
            case 'n':
                args->mean_output = optarg;
                break;
            case 'T':
                args->test_matrix_prefix = optarg;
                break;
            case 'x':
                value = atoi(optarg);
                if (value <= 0) {
                    print_usage(argv[0]);
                    return NULL;
                }
                args->x_batch = (unsigned int) value;
                break;
            case 't':
                value = atoi(optarg);
                if (value <= 0) {
                    print_usage(argv[0]);
                    return NULL;
                }
                args->t_batch = (unsigned int) value;
                break;
            case 'M':
                value = atoi(optarg);
                if (value <= 0) {
                    print_usage(argv[0]);
                    return NULL;
                }
                args->m = (unsigned int) value;
                break;
            case 'N':
                value = atoi(optarg);
                if (value <= 0) {
                    print_usage(argv[0]);
                    return NULL;
                }
                args->n = (unsigned int) value;
                break;
            case 'i':
                value = atoi(optarg);
                if (value <= 0) {
                    print_usage(argv[0]);
                    return NULL;
                }
                args->iterations = (unsigned int) value;
                break;
            default:
                break;
        }
    }

    option_index = 0;
    while (optind < argc && option_index < ARGS_COUNT) {
        pointers[option_index++] = argv[optind++];

    }
    if (option_index < ARGS_COUNT) {
        print_usage(argv[0]);
        return NULL;
    }

    args->train_matrix_prefix = pointers[0];
    args->x_output = pointers[1];
    args->t_output = pointers[2];
    return args;
}