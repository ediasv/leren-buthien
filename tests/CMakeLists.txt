find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Executável dos testes
add_executable(
  tests
  MainTest.cpp
  IDTest.cpp
  CaminhoRecursosTest.cpp
)

# Adiciona o diretorio 'include/'
target_include_directories(
  tests
  PRIVATE
  ${PROJECT_INCLUDE_DIR}
  ${GTEST_INCLUDE_DIRS}
)

# Linka todos os arquivos de implementacao do jogo aos testes
target_link_libraries(
  tests
  PUBLIC
  GTest::GTest
  GTest::Main
  ${CMAKE_THREAD_LIBS_INIT}
  jogo
)

target_compile_options(
tests
PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

include(GoogleTest)
gtest_discover_tests(tests)
