if(GMX_GPU)
    file(GLOB CUDA_NB_SOURCES *.cu)
    CUDA_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
    CUDA_ADD_LIBRARY(nbnxn_cuda STATIC ${CUDA_NB_SOURCES}
            OPTIONS
            RELWITHDEBINFO -g
            DEBUG -g -D_DEBUG_=1)
    target_link_libraries(nbnxn_cuda cuda_tools)
endif()
