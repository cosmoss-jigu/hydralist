set(CMAKE_CXX_FLAGS_RELEASE "-O3")
file(GLOB artsrc "*.cpp" "*.h")

add_library(art STATIC ${artsrc})

target_include_directories(art PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
INSTALL(TARGETS art 
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}
        )

