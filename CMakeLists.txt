cmake_minimum_required(VERSION 3.20)
project(YmirHelga LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

# Configurações do projeto
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Padrões de compilação
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Diretório de saída dos binários
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Opção para construir bibliotecas compartilhadas
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Diretório de includes do projeto
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
include_directories("${PROJECT_INCLUDE_DIR}")

find_package(SFML 2 CONFIG REQUIRED COMPONENTS graphics window system)

# Subdiretórios do projeto
add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(
  DIRECTORY "${CMAKE_SOURCE_DIR}/assets/"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/ymir-helga/assets"
)

install(
  FILES "${CMAKE_SOURCE_DIR}/packaging/appimage/io.github.ediasv.YmirHelga.desktop"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)

install(
  FILES "${CMAKE_SOURCE_DIR}/assets/Personagens/Ymir.png"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps"
  RENAME "io.github.ediasv.YmirHelga.png"
)

install(
  FILES "${CMAKE_SOURCE_DIR}/packaging/appimage/io.github.ediasv.YmirHelga.metainfo.xml"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo"
)
