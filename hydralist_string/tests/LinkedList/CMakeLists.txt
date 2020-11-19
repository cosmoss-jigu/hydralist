file(GLOB SRCS *.cpp)
ADD_EXECUTABLE(linkedListTest ${SRCS})

TARGET_LINK_LIBRARIES(
    linkedListTest
    hydraList
    gtest
    numa
    art
    tbb
)
add_test(NAME linkedListTest
         COMMAND linkedListTest)
