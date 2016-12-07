cmake_minimum_required(VERSION 3.6)

project(CuMF)

find_package(CUDA QUIET REQUIRED)

set(
        CUDA_NVCC_FLAGS
        ${CUDA_NVCC_FLAGS}
        -Xcompiler -DADD_ -gencode arch=compute_35,code=sm_35 -gencode arch=compute_35,code=compute_35
)


set(SOURCE_FILES
        als.cu
        cg.cu
        device_utilities.cu
        host_utilities.cpp
        utils/argparse.cpp
        quokka_als
        )

cuda_add_executable(
        quokka_als
        ${SOURCE_FILES}
)
cuda_add_cublas_to_target(quokka_als)
target_link_libraries(
        quokka_als
        ${CUDA_cusparse_LIBRARY}
)
