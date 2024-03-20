set(PROJECT_NAME simulator)

set(PROJECT_HEADER
    include/batch_test.h
    include/threadpool.h
)

set(PROJECT_SOURCE
    src/batch_test.cpp)

set(ALL_FILES ${PROJECT_HEADER} ${PROJECT_SOURCE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} ${ALL_FILES} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC algorithm)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_include_directories(${PROJECT_NAME} PUBLIC include)