file(GLOB SRCS *.cpp)
ADD_EXECUTABLE(opLogTest ${SRCS})

TARGET_LINK_LIBRARIES(
    opLogTest
    hydraList
    gtest
    numa
    art
    tbb
)

add_test(NAME opLogTest
        COMMAND opLogTest)
