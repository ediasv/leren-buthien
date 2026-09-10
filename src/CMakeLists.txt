# ---------------------------------------------------------
# Biblioteca Jogo

add_library(jogo
  # Raiz do projeto
  Jogo.cpp
  Ente.cpp
  CaminhoRecursos.cpp
  State.cpp
  Mediator.cpp
  ObserverFase.cpp

  # Diretorio Observer
  Observer/Observer.cpp
  Observer/Subject.cpp

  # Diretorio Gerenciadores
  Gerenciadores/GerenciadorColisoes.cpp
  Gerenciadores/GerenciadorEstados.cpp
  Gerenciadores/GerenciadorEventos.cpp
  Gerenciadores/GerenciadorGrafico.cpp
  Gerenciadores/GerenciadorInput.cpp
  Gerenciadores/GerenciadorColisoes.cpp

  # Diretorio Controladores
  Controladores/ControladorJogador.cpp
  Controladores/ControladorMenu.cpp

  # Diretorio Entidades
  Entidades/Entidade.cpp
  Entidades/Projetil.cpp

  # Entidades::Personagens
  Entidades/Personagens/Esqueleto.cpp
  Entidades/Personagens/Inimigo.cpp
  Entidades/Personagens/Jogador.cpp
  Entidades/Personagens/Mago.cpp
  Entidades/Personagens/Personagem.cpp
  Entidades/Personagens/Slime.cpp

  # Diretorio Fabrica
  Fabrica/FabricaEntidadesCaverna.cpp
  Fabrica/FabricaEntidades.cpp
  Fabrica/FabricaEntidadesPlanicie.cpp
  Fabrica/FabricaProjeteis.cpp

  # Entidades::Obstaculos
  Entidades/Obstaculos/Espinho.cpp
  Entidades/Obstaculos/Gosma.cpp
  Entidades/Obstaculos/Obstaculo.cpp
  Entidades/Obstaculos/Plataforma.cpp

  # Diretorio Fases
  Fases/Caverna.cpp
  Fases/Fase.cpp
  Fases/Planicie.cpp

  #Diretorio Menu
  Menu/Menu.cpp
  Menu/MenuPrincipal.cpp
  Menu/MenuPausa.cpp
  Menu/Botao.cpp
  Menu/GameOver.cpp

  # Diretorio Listas
  Listas/ListaEntidade.cpp
)

target_compile_definitions(jogo
  PRIVATE
    YMIR_HELGA_INSTALL_ASSET_DIR="${CMAKE_INSTALL_FULL_DATADIR}/ymir-helga/assets"
    YMIR_HELGA_DEVELOPMENT_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)

target_link_libraries(jogo
  PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
)

# ---------------------------------------------------------
# Executável principal

add_executable(ymir-helga
  Main.cpp
)

target_link_libraries(ymir-helga
  PRIVATE
    jogo
)

install(TARGETS ymir-helga
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
