file(GLOB SRCS *.cpp)
ADD_EXECUTABLE(workerThreadTest ${SRCS})

TARGET_LINK_LIBRARIES(
    workerThreadTest
    hydraList
    gtest
    numa
    art
    tbb
)

add_test(NAME workerThreadTest
        COMMAND workerThreadTest)
