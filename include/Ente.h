#pragma once
#include "GerenciadorGrafico.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <iostream>

class Gerenciador_Grafico;

class Ente {
protected:
  int id;
  static Gerenciador_Grafico *pGG;
  sf::Texture *texture;

public:
  Ente(Gerenciador_Grafico *pGG, sf::Texture *Texture, int id = 0);
  ~Ente();
  virtual void executar() = 0;
  virtual void desenhar() = 0;
  static void setGerenciadorGrafico(Gerenciador_Grafico *pGerenciador);
};
