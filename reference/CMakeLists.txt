add_library(ginkgo_reference $<TARGET_OBJECTS:ginkgo_reference_device> "")
target_sources(ginkgo_reference
    PRIVATE
        base/version.cpp
        matrix/coo_kernels.cpp
        matrix/csr_kernels.cpp
        matrix/dense_kernels.cpp
        matrix/ell_kernels.cpp
        matrix/hybrid_kernels.cpp
        matrix/sellp_kernels.cpp
        preconditioner/jacobi_kernels.cpp
        solver/bicgstab_kernels.cpp
        solver/cg_kernels.cpp
        solver/cgs_kernels.cpp
        solver/fcg_kernels.cpp
        solver/gmres_kernels.cpp
        solver/ir_kernels.cpp
        stop/criterion_kernels.cpp
        stop/residual_norm_reduction_kernels.cpp)

ginkgo_compile_features(ginkgo_reference)
ginkgo_default_includes(ginkgo_reference)
ginkgo_install_library(ginkgo_reference reference)
target_compile_options(ginkgo_reference PRIVATE "${GINKGO_COMPILER_FLAGS}")

# Ensures that there are no circular dependency issues
if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(ginkgo_reference PUBLIC ginkgo)
endif()

if(GINKGO_BUILD_TESTS)
    add_subdirectory(test)
endif()
