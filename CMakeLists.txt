CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(chunky VERSION 0.1)

set(CHUNKY_LIBS stdc++ m)
set(CHUNKY_SRC external/libdicey/dice.cpp chunky.cpp chunky.h)
enable_testing()

ADD_EXECUTABLE(basic_test tests/basic_test.cpp ${CHUNKY_SRC})
TARGET_INCLUDE_DIRECTORIES(basic_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(basic_test ${CHUNKY_LIBS})
ADD_TEST(basic_test basic_test)

ADD_EXECUTABLE(chunkgen chunkgen.cpp ${CHUNKY_SRC})
TARGET_INCLUDE_DIRECTORIES(chunkgen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(chunkgen ${CHUNKY_LIBS})
ADD_TEST(chunkgen chunkgen_test1)
