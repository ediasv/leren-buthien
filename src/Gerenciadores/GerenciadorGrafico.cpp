#include "Gerenciadores/GerenciadorGrafico.h"

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <iostream>

#include "CaminhoRecursos.h"
#include "Ente.h"

namespace Gerenciadores {

GerenciadorGrafico *GerenciadorGrafico::instancia = nullptr;

GerenciadorGrafico::GerenciadorGrafico() : deltaTempo(0.f) {
  pJanela = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode()),
                                 "Ymir e Helga", sf::Style::Fullscreen);
  pJanela->setVerticalSyncEnabled(true);
  pJanela->setFramerateLimit(30);
  pJanela->requestFocus();

  larguraJanela = pJanela->getSize().x;
  alturaJanela = pJanela->getSize().y;
  superPixel = new sf::Font;

  carregarFonte("/assets/SuperPixel-m2L8j.ttf");

  relogio.restart();
}

GerenciadorGrafico::~GerenciadorGrafico() {
  pJanela = nullptr;
  delete instancia;
}

GerenciadorGrafico *GerenciadorGrafico::getInstancia() {
  if (!instancia) {
    instancia = new GerenciadorGrafico();
  }
  return instancia;
}

bool GerenciadorGrafico::janelaAberta() { return pJanela->isOpen(); }

void GerenciadorGrafico::display() { pJanela->display(); }

void GerenciadorGrafico::clear() { pJanela->clear(); }

void GerenciadorGrafico::desenharEnte(Ente *pE) {
  if (pJanela && pE) {
    pJanela->draw(pE->getSprite());
  } else {
    std::cerr
        << "erro: parametro invalido: Gerenciador_Grafico::desenharEnte()\n";
    exit(EXIT_FAILURE);
  }
}

void GerenciadorGrafico::fecharJanela() {
  pJanela->close();
  pJanela = nullptr;
}

void GerenciadorGrafico::carregarFonte(const std::string &path) {
  const std::filesystem::path caminhoCompleto =
      Recursos::resolverCaminhoRecurso(path);

  if (!superPixel->loadFromFile(caminhoCompleto.string())) {
    std::cerr << "erro: não foi possível carregar a fonte " << caminhoCompleto
              << '\n';
    exit(EXIT_FAILURE);
  }
}

sf::Font *GerenciadorGrafico::getFonte() const {
  if (superPixel == nullptr) {
    std::cerr << "Erro em getFonte() => nullptr em SuperPixel!\n";
    exit(EXIT_FAILURE);
  }
  return superPixel;
}

sf::RenderWindow *GerenciadorGrafico::getJanela() const { return pJanela; }

void GerenciadorGrafico::atualizaDeltaTempo() {
  deltaTempo = relogio.restart().asMilliseconds();
}

const float GerenciadorGrafico::getDeltaTempo() const { return deltaTempo; }

const float GerenciadorGrafico::getLarguraJanela() const {
  return larguraJanela;
}
const float GerenciadorGrafico::getAlturaJanela() const { return alturaJanela; }

}  // namespace Gerenciadores
