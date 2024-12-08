#pragma once

#include "Gerenciadores/GerenciadorGrafico.h"
#include <SFML/Window/Event.hpp>

namespace Gerenciadores {

class Gerenciador_Eventos {
private:
  sf::Event evento;
  static Gerenciador_Eventos *instancia; // Singleton
  Gerenciador_Eventos();

public:
  ~Gerenciador_Eventos();
  static Gerenciador_Eventos *getInstancia();
  sf::Event &getEvento();
  void processaEventos(Gerenciador_Grafico *pGG);
};

} // namespace Gerenciadores