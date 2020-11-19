file (GLOB HydraList_SRCS "*.h" "*.cpp")
add_library(hydraList STATIC
    ${HydraList_SRCS}
)
target_include_directories(hydraList PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
add_executable (main main.cpp)
target_link_libraries (main hydraList numa jemalloc art)
INSTALL(TARGETS hydraList 
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}
        )
