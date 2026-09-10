#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

#include "CaminhoRecursos.h"

namespace {

class VariavelAmbienteTemporaria {
 public:
  VariavelAmbienteTemporaria(const char* nome, const char* valor) : nome(nome) {
    salvarValorAnterior();
    definir(valor);
  }

  explicit VariavelAmbienteTemporaria(const char* nome) : nome(nome) {
    salvarValorAnterior();
    remover();
  }

  ~VariavelAmbienteTemporaria() {
    if (anterior) {
      definir(anterior->c_str());
    } else {
      remover();
    }
  }

  VariavelAmbienteTemporaria(const VariavelAmbienteTemporaria&) = delete;
  VariavelAmbienteTemporaria& operator=(
      const VariavelAmbienteTemporaria&) = delete;

 private:
  void salvarValorAnterior() {
    if (const char* valorAnterior = std::getenv(nome.c_str())) {
      anterior = valorAnterior;
    }
  }

  void definir(const char* valor) {
#ifdef _WIN32
    _putenv_s(nome.c_str(), valor);
#else
    setenv(nome.c_str(), valor, 1);
#endif
  }

  void remover() {
#ifdef _WIN32
    _putenv_s(nome.c_str(), "");
#else
    unsetenv(nome.c_str());
#endif
  }

  std::string nome;
  std::optional<std::string> anterior;
};

TEST(CaminhoRecursosTest, HonraDiretorioConfigurado) {
  VariavelAmbienteTemporaria ambiente("YMIR_HELGA_ASSET_DIR",
                                      "/tmp/ymir helga/assets");

  EXPECT_EQ(Recursos::resolverCaminhoRecurso("Menu.png"),
            std::filesystem::path("/tmp/ymir helga/assets/Menu.png"));
}

TEST(CaminhoRecursosTest, RejeitaDiretorioAssetsRelativo) {
  VariavelAmbienteTemporaria ambiente("YMIR_HELGA_ASSET_DIR", "assets");

  EXPECT_THROW(Recursos::resolverCaminhoRecurso("Menu.png"),
               std::invalid_argument);
}

TEST(CaminhoRecursosTest, RemoveBarraEPrefixoAssets) {
  VariavelAmbienteTemporaria ambiente("YMIR_HELGA_ASSET_DIR",
                                      "/tmp/ymir-helga-assets");
  const std::filesystem::path esperado =
      "/tmp/ymir-helga-assets/Mapas/planicie.txt";

  EXPECT_EQ(
      Recursos::resolverCaminhoRecurso("/assets/Mapas/planicie.txt"),
      esperado);
  EXPECT_EQ(Recursos::resolverCaminhoRecurso("assets/Mapas/planicie.txt"),
            esperado);
}

TEST(CaminhoRecursosTest, CaminhoAbsolutoNaoSubstituiDiretorioBase) {
  VariavelAmbienteTemporaria ambiente("YMIR_HELGA_ASSET_DIR",
                                      "/tmp/ymir-helga-assets");

  EXPECT_EQ(Recursos::resolverCaminhoRecurso("/Menu.png"),
            std::filesystem::path("/tmp/ymir-helga-assets/Menu.png"));
}

TEST(CaminhoRecursosTest, ResolveTexturaFonteEMapaDentroDoAppDir) {
  VariavelAmbienteTemporaria semDiretorioConfigurado(
      "YMIR_HELGA_ASSET_DIR");
  VariavelAmbienteTemporaria appDir("APPDIR", "/tmp/Ymir Helga.AppDir");
  const std::filesystem::path diretorioAssets =
      "/tmp/Ymir Helga.AppDir/usr/share/ymir-helga/assets";

  EXPECT_EQ(Recursos::resolverCaminhoRecurso("/assets/Menu.png"),
            diretorioAssets / "Menu.png");
  EXPECT_EQ(Recursos::resolverCaminhoRecurso("SuperPixel-m2L8j.ttf"),
            diretorioAssets / "SuperPixel-m2L8j.ttf");
  EXPECT_EQ(Recursos::resolverCaminhoRecurso("assets/Mapas/caverna.txt"),
            diretorioAssets / "Mapas/caverna.txt");
}

TEST(CaminhoRecursosTest, DiretorioConfiguradoTemPrecedenciaSobreAppDir) {
  VariavelAmbienteTemporaria appDir("APPDIR", "/tmp/AppDir");
  VariavelAmbienteTemporaria ambiente("YMIR_HELGA_ASSET_DIR",
                                      "/tmp/assets-configurados");

  EXPECT_EQ(Recursos::resolverCaminhoRecurso("Menu.png"),
            std::filesystem::path("/tmp/assets-configurados/Menu.png"));
}

TEST(CaminhoRecursosTest, RejeitaCaminhoQueEscapaDosAssets) {
  VariavelAmbienteTemporaria ambiente("YMIR_HELGA_ASSET_DIR",
                                      "/tmp/ymir-helga-assets");

  EXPECT_THROW(Recursos::resolverCaminhoRecurso("../Menu.png"),
               std::invalid_argument);
}

}  // namespace
