# Base Defense - Game

Base Defense é um jogo de defesa onde o jogador deve proteger a base de uma invasão alienígena. O jogador controla uma nave espacial que pode se mover livremente pela tela, atirar projéteis e coletar itens para melhorar suas habilidades.

## Requisitos

- **SFML (Simple and Fast Multimedia Library)** - Biblioteca gráfica utilizada para a criação do jogo.
- **Compilador C++** (por exemplo, GCC ou Clang).
- **Sistema Operacional**: Testado em Windows, mas pode funcionar em outros sistemas com a instalação adequada da SFML.

## Funcionalidades

- **Movimento do jogador**: O jogador pode mover sua nave com o botão direito do mouse e mirar usando o cursor do mouse.
- **Atirar projéteis**: O jogador pode atirar projéteis com a tecla **Q**. A quantidade de munição é limitada.
- **Inimigos**: Inimigos se aproximam da base e tentam destruí-la. Quando atingem a base, causam danos.
- **Coletáveis**: Munição e itens de cura caem aleatoriamente quando inimigos são destruídos.
- **Respawn do jogador**: O jogador morre quando toca em inimigos, mas pode renascer após 5 segundos.
- **Aumento de dificuldade**: O intervalo de spawn dos inimigos diminui conforme o tempo passa.
- **Música e Efeitos Sonoros**: Música de fundo e efeitos sonoros para diversas ações no jogo.

## Instruções de Compilação

### Pré-requisitos

1. **Instalar a SFML**:
   - Se você ainda não tiver a SFML instalada, siga as instruções da [documentação oficial](https://www.sfml-dev.org/download.php) para sua plataforma.

2. **Instalar compilador C++**:
   - **Windows**: Utilize o **MinGW** ou **MSVC** (Microsoft Visual Studio).
   - **Linux**: Use `g++` ou `clang++`.
   - **MacOS**: Utilize o `clang++`.

### Compilação no Linux/MacOS:

.```bash
# Instalar dependências (caso necessário)
sudo apt-get install libsfml-dev

# Compilando o código-fonte
g++ -o base_defense main.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Executando o jogo
./base_defense

### Compilação no Windows (via MinGW):

# Compilando com o MinGW
g++ -o base_defense.exe main.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Executando o jogo
base_defense.exe

### Estrutura do Código
Componentes Principais:

  Entidades (Entities):
      Hero: O jogador (nave espacial) que controla o herói.
      Inimigos (Enemies): Entidades que atacam a base.
      Projetéis (Projectiles): Disparos feitos pelo herói para destruir inimigos.
      Itens (AmmoBag, HealBag): Itens que caem dos inimigos mortos.

  Movimento: A função moveEntity controla o movimento do jogador, inimigos e a nave de acordo com as entradas de teclado e mouse.

  Spawn de Inimigos: Inimigos são gerados a intervalos regulares, e sua dificuldade aumenta ao longo do tempo.

  Interações:
      Colisão entre projéteis e inimigos, onde o inimigo é destruído e pode dropar itens.
      Colisão entre a nave do jogador e inimigos, onde o jogador perde saúde ou morre.
      Coleta de munição e itens de cura ao colidir com os respectivos objetos.

  Música e Efeitos Sonoros: Utiliza a SFML para tocar música de fundo e efeitos sonoros, como tiros, morte de inimigos e dano à base.

