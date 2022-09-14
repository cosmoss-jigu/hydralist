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
        enable_testing()

        # 単体テストの定義
        add_executable(MyLibTest test.cpp)
        target_link_libraries(MyLibTest PRIVATE mylib gtest_main)
        
        # CTestへ単体テストを登録
        include(GoogleTest)
        gtest_discover_tests(MyLibTest)