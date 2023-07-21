cmake_minimum_required(VERSION 3.20)
project(SearchServer)

set(CMAKE_CXX_STANDARD 17)

set(FILES_INCLUDE
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
        ./include/test_framework.h)

set(FILES_SOURCE
        ./src/process_queries.cpp
        ./src/document.cpp
        ./src/process_queries.cpp
        ./src/read_input_functions.cpp
        ./src/request_queue.cpp
        ./src/search_server.cpp
        ./src/string_processing.cpp
        ./src/test_example_functions.cpp
        ./src/document.cpp)

set(FILE_MAIN main.cpp)

add_executable(SearchServer ${FILE_MAIN}  ${FILES_INCLUDE} ${FILES_SOURCE} )

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

target_link_libraries(SearchServer -ltbb -lpthread ${SYSTEM_LIBS})
