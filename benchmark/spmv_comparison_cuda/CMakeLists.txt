if (GINKGO_BUILD_CUDA)
    add_executable(spmv_comparison_cuda spmv_comparison_cuda.cpp)
    target_link_libraries(spmv_comparison_cuda ginkgo ${CUDA_RUNTIME_LIBS}
        ${CUBLAS} ${CUSPARSE})
    target_include_directories(spmv_comparison_cuda PRIVATE ${PROJECT_SOURCE_DIR})
    target_include_directories(spmv_comparison_cuda SYSTEM PRIVATE ${CUDA_INCLUDE_DIRS})
    if(CUDA_VERSION GREATER_EQUAL "92")
        target_compile_definitions(spmv_comparison_cuda PRIVATE ALLOWMP=1)
    endif()
endif()

