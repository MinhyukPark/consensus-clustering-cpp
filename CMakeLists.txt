cmake_minimum_required(VERSION 3.23)
#[[ In-source build check ]]
if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed.")
    message(WARNING "Remove CMakeFiles/ and CMakeCache.txt in the\
            source root before building again.")
else()
    #[[ Set project version ]]
    project(consensus_clustering)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/external_libs/lib/cmake/igraph)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/external_libs/lib/cmake/libleidenalg)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/external_libs/lib64/cmake/igraph)
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/external_libs/lib64/cmake/libleidenalg)
    find_package(igraph 0.10 CONFIG REQUIRED)
    find_package(libleidenalg 0.11 CONFIG REQUIRED)
    find_package(OpenMP REQUIRED)

    set(CONSENSUS_CLUSTERING_MAJOR_VERSION 0)
    set(CONSENSUS_CLUSTERING_MINOR_VERSION 1)
    set(CONSENSUS_CLUSTERING_PATCH_VERSION 0)
    set(CONSENSUS_CLUSTERING_VERSION
        ${CONSENSUS_CLUSTERING_MAJOR_VERSION}.
        ${CONSENSUS_CLUSTERING_MINOR_VERSION}.
        ${CONSENSUS_CLUSTERING_PATCH_VERSION})


    #[[ CONSTANTS ]]
    # Compiler options
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd lsp
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -g -O0 -ggdb")
    # File paths
    set(SOURCE_MAIN ${CMAKE_SOURCE_DIR}/src/main.cpp)
    set(SOURCE_INTERNAL_LIBS
        ${CMAKE_SOURCE_DIR}/src/simple_consensus.cpp
        ${CMAKE_SOURCE_DIR}/src/multi_resolution_consensus.cpp
        ${CMAKE_SOURCE_DIR}/src/threshold_consensus.cpp
        ${CMAKE_SOURCE_DIR}/src/simple_ensemble_clustering.cpp
        ${CMAKE_SOURCE_DIR}/src/ensemble_consensus.cpp
        ${CMAKE_SOURCE_DIR}/src/consensus.cpp)
    #[[ CONSTANTS END ]]

    #[[ Define the output binary ]]
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    add_executable(consensus_clustering ${SOURCE_MAIN})

    #[[ Link internal libraries ]]
    add_library(internal_libs ${SOURCE_INTERNAL_LIBS})
    target_include_directories(internal_libs
        PUBLIC ${CMAKE_SOURCE_DIR}/includes
        PUBLIC ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(internal_libs PUBLIC igraph::igraph)
    target_link_libraries(internal_libs PUBLIC libleidenalg::libleidenalg)
    target_link_libraries(internal_libs PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(consensus_clustering PUBLIC internal_libs)

    #[[ Link libraries ]]
    add_subdirectory(libs)
    target_link_libraries(consensus_clustering PUBLIC external_libs)

    #[[ Link external libraries ]]
endif("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
