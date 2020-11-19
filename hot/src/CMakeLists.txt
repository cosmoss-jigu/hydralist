#add_executable(sample "main.cpp")
#target_link_libraries(sample hot-rowex-lib content-helpers-lib)
add_library(hot-rowex STATIC main.cpp)
target_link_libraries(hot-rowex hot-rowex-lib content-helpers-lib)
target_include_directories(hot-rowex
	PUBLIC ${CMAKE_SOURCE_DIR}/src
	PUBLIC hot/rowex
	PUBLIC idx/contenthelpers
	)
INSTALL(TARGETS hot-rowex
	ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_SOURCE_DIR}/include
	)

