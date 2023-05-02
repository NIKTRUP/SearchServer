cmake_minimum_required(VERSION 3.20)
project(SearchServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(SearchServer main.cpp
        ./include/concurrent_map.h
        ./include/document.h
        ./include/log_duration.h
        ./include/paginator.h
        ./include/process_queries.h
        ./include/read_input_functions.h
        ./include/request_queue.h
        ./include/search_server.h
        ./include/string_processing.h
        ./include/test_example_functions.h
        ./include/test_framework.h

        ./src/process_queries.cpp
        ./src/document.cpp
        ./src/process_queries.cpp
        ./src/read_input_functions.cpp
        ./src/request_queue.cpp
        ./src/search_server.cpp
        ./src/string_processing.cpp
        ./src/test_example_functions.cpp src/document.cpp)

target_link_libraries(SearchServer -ltbb -lpthread)
