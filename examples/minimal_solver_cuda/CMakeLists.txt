add_executable(minimal_solver_cuda minimal_solver_cuda.cpp)
target_link_libraries(minimal_solver_cuda ginkgo)
target_include_directories(minimal_solver_cuda PRIVATE ${PROJECT_SOURCE_DIR})
configure_file(data/A.mtx data/A.mtx COPYONLY)
configure_file(data/b.mtx data/b.mtx COPYONLY)
configure_file(data/x0.mtx data/x0.mtx COPYONLY)
