file(GLOB SRCS *.cpp)
MESSAGE($SRCS)
ADD_EXECUTABLE(hydraListTest ${SRCS})

TARGET_LINK_LIBRARIES(
        hydraListTest
        tbb
        hydraList
        gtest
        numa
        art
        tbb
)

add_test(NAME hydraListTest
        COMMAND hydraListTest)
