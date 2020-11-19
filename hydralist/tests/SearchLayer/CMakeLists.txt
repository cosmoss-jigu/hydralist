file(GLOB SRCS *.cpp)
MESSAGE($SRCS)
ADD_EXECUTABLE(searchLayerTest ${SRCS})

TARGET_LINK_LIBRARIES(
        searchLayerTest
        tbb
        hydraList
        gtest
        numa
        art
        tbb
)

add_test(NAME searchLayerTest
        COMMAND searchLayerTest)
