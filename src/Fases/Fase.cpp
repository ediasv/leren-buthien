#include "Fases/Fase.h"

#include <SFML/System/Vector2.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "CaminhoRecursos.h"
#include "Entidades/Entidade.h"
#include "Entidades/Personagens/Inimigo.h"
#include "Entidades/Personagens/Jogador.h"
#include "Entidades/Personagens/Personagem.h"
#include "Entidades/Projetil.h"
#include "Gerenciadores/GerenciadorColisoes.h"
#include "IDs.h"
#include "Listas/ListaEntidades.h"
#include "ObserverFase.h"

using namespace Entidades;

namespace Fases {

Fase::Fase(ID id, bool mp)
    : Ente(id),
      States::State(id),
      listaObstaculos(),
      listaJogadores(),
      listaInimigos(),
      listaProjeteis(),
      mp(mp),
      pGC(Gerenciadores::GerenciadorColisoes::getInstancia()),
      thisObs(nullptr),
      pFE(nullptr) {
  listaObstaculos.limpar();
  listaJogadores.limpar();
  listaInimigos.limpar();
  listaProjeteis.limpar();

  Personagens::Jogador::resetJog();

  Entidades::Personagens::Personagem::setFase(this);
  thisObs = new ObservadorFase(this);
}

Fase::~Fase() {
  listaObstaculos.limpar();
  listaJogadores.limpar();
  listaInimigos.limpar();
  listaProjeteis.limpar();
  pGC = nullptr;
  delete thisObs;
  thisObs = nullptr;
}

void Fase::incluirNaLista(Entidade* novaEntidade) {
  if (!novaEntidade) {
    std::cerr << "erro: Fase::incluirNaLista(...) => novaEntidade == nullptr\n";
    exit(EXIT_FAILURE);
  }

  ID id = novaEntidade->getId();

  if (ehJogador(id)) {
    listaJogadores.incluir(novaEntidade);
    Personagens::Inimigos::Inimigo::adicionarJogador(
        dynamic_cast<Personagens::Jogador*>(novaEntidade));

  } else if (ehInimigo(id)) {
    listaInimigos.incluir(novaEntidade);

  } else if (ehObstaculo(id)) {
    listaObstaculos.incluir(novaEntidade);

  } else if (ehProjetil(id)) {
    listaProjeteis.incluir(novaEntidade);
  }
}

void Fase::removerProjetil(Projetil* projetil) {
  if (!projetil) {
    std::cerr << "erro: Fase::removerProjetil(...) => projetil == nullptr\n";
    exit(EXIT_FAILURE);
  }
  pGC->removerEnt(projetil);
  listaProjeteis.deletar(projetil);
}

void Fase::adicionarProjetil(Entidades::Projetil* novoProjetil) {
  if (!novoProjetil) {
    std::cerr
        << "erro: Fase::adicionarProjetil(...) => novoProjetil == nullptr\n";
    exit(EXIT_FAILURE);
  }

  incluirNaLista(novoProjetil);
  novoProjetil->setFase(this);
}

void Fase::criarMapa(const std::string path) {
  if (!pFE) {
    exit(EXIT_FAILURE);
  }

  const std::filesystem::path caminhoCompleto =
      Recursos::resolverCaminhoRecurso(path);
  std::ifstream arquivoMapa(caminhoCompleto);

  if (!arquivoMapa.is_open()) {
    std::cerr << "erro: não foi possível abrir o mapa " << caminhoCompleto
              << '\n';
    exit(EXIT_FAILURE);
  }

  std::string linha;
  Entidade* novaEntidade;
  for (int j = 0; std::getline(arquivoMapa, linha); j++) {
    for (int i = 0; i < linha.size(); i++) {
      if (linha[i] != ' ') {
        novaEntidade =
            pFE->criarEntidade(linha[i], sf::Vector2f(i * 16, j * 16));
        if (novaEntidade != nullptr) {
          incluirNaLista(novaEntidade);
        }
      }
    }
  }

  arquivoMapa.close();
}
void Fase::notificarMorreu(Entidades::Entidade* pEnt) {
  if (!pEnt) {
    std::cerr << "erro: Fase::notificarMorreu(...) => pEnt == nullptr\n";
    exit(EXIT_FAILURE);
  }

  if (pEnt->getId() == ID::IDjogador) {
    pGC->removerEnt(pEnt);
    listaJogadores.deletar(pEnt);
  } else {
    pGC->removerEnt(pEnt);
    listaInimigos.deletar(pEnt);
  }
}

void Fase::limparListas() {
  listaProjeteis.limpar();
  listaJogadores.limpar();
  listaObstaculos.limpar();
  listaInimigos.limpar();
  pGC->limparEntidades();
}

const bool Fase::getMp() {
  return mp;
}

}  // namespace Fases
