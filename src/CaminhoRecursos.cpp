#include "CaminhoRecursos.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

#ifndef YMIR_HELGA_INSTALL_ASSET_DIR
#error "YMIR_HELGA_INSTALL_ASSET_DIR deve ser definido pelo CMake"
#endif

#ifndef YMIR_HELGA_DEVELOPMENT_ASSET_DIR
#error "YMIR_HELGA_DEVELOPMENT_ASSET_DIR deve ser definido pelo CMake"
#endif

namespace {

std::filesystem::path normalizarCaminhoRelativo(
    const std::filesystem::path& caminho) {
  std::string texto = caminho.generic_string();

  while (!texto.empty() && texto.front() == '/') {
    texto.erase(texto.begin());
  }

  while (texto.rfind("./", 0) == 0) {
    texto.erase(0, 2);
  }

  if (texto == "assets") {
    texto.clear();
  } else if (texto.rfind("assets/", 0) == 0) {
    texto.erase(0, 7);
  }

  const std::filesystem::path relativo =
      std::filesystem::path(texto).lexically_normal();

  for (const std::filesystem::path& componente : relativo) {
    if (componente == "..") {
      throw std::invalid_argument(
          "o caminho do recurso não pode sair do diretório de assets");
    }
  }

  return relativo.relative_path();
}

const char* obterVariavelAmbiente(const char* nome) {
  const char* valor = std::getenv(nome);
  return valor != nullptr && valor[0] != '\0' ? valor : nullptr;
}

}  // namespace

namespace Recursos {

std::filesystem::path resolverCaminhoRecurso(
    const std::filesystem::path& caminhoRelativo) {
  std::filesystem::path diretorioAssets;

  if (const char* configurado =
          obterVariavelAmbiente("YMIR_HELGA_ASSET_DIR")) {
    diretorioAssets = configurado;
  } else if (const char* appDir = obterVariavelAmbiente("APPDIR")) {
    diretorioAssets =
        std::filesystem::path(appDir) / "usr/share/ymir-helga/assets";
  } else {
    const std::filesystem::path diretorioInstalado =
        YMIR_HELGA_INSTALL_ASSET_DIR;

    if (std::filesystem::exists(diretorioInstalado)) {
      diretorioAssets = diretorioInstalado;
    } else {
      diretorioAssets = YMIR_HELGA_DEVELOPMENT_ASSET_DIR;
    }
  }

  if (!diretorioAssets.is_absolute()) {
    throw std::invalid_argument(
        "o diretório de assets configurado deve ser um caminho absoluto");
  }

  return (diretorioAssets / normalizarCaminhoRelativo(caminhoRelativo))
      .lexically_normal();
}

}  // namespace Recursos
