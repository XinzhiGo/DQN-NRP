set(PROJECT_NAME NurseRostering)

set(${PROJECT_NAME}_HEADER
    include/DebugFlag.h
    include/INRC2.h
    include/NurseRostering.h
    include/Solution.h
    include/Solver.h
    include/utility.h
)

set(${PROJECT_NAME}_SOURCE
    src/INRC2.cpp
    src/NurseRostering.cpp
    src/Solution.cpp
    src/Solver.cpp
    src/utility.cpp
)

set(${PROJECT_NAME}_ALL_FILES ${${PROJECT_NAME}_HEADER} ${${PROJECT_NAME}_SOURCE})

find_package(spdlog REQUIRED)
find_package(Torch REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_library(algorithm SHARED ${${PROJECT_NAME}_ALL_FILES})
target_link_libraries(algorithm PUBLIC spdlog::spdlog)
target_link_libraries(algorithm PRIVATE ${TORCH_LIBRARIES})
target_include_directories(algorithm PUBLIC ./include ${TORCH_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE algorithm)
