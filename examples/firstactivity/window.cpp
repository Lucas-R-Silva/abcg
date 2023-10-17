#include "window.hpp"

void Window::onEvent(SDL_Event const &event) {
    // Lidar com eventos, como teclas pressionadas ou cliques do mouse
    if (event.type == SDL_KEYDOWN) {
        // Verifique se uma tecla foi pressionada
        if (event.key.keysym.sym == SDLK_SPACE) {
            // Se a barra de espaço foi pressionada, faça o pássaro pular
            bird.jump();
        }
    }
}

void Window::onCreate() {
    // Configurar o programa OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Carregar shaders (suponha que você tenha funções para carregar shaders)
    GLuint vertexShader = loadShader("bird.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader("bird.frag", GL_FRAGMENT_SHADER);
    m_program = createProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Carregar texturas (suponha que você tenha funções para carregar texturas)
    m_birdTexture = loadTexture("bird_texture.png");

    // Inicializar objetos do jogo
    m_bird.initializeGL(m_program);
    m_bird.setTexture(m_birdTexture); // Atribua a textura ao Bird
    m_bamboo.initializeGL(m_program);

    // Defina a matriz de projeção (suponha que você tenha funções para configurar a matriz)
    m_projectionMatrix = glm::ortho(0.0f, 600.0f, 0.0f, 600.0f, -1.0f, 1.0f);
}


void Window::onUpdate() {
    if (m_gameData.m_state == State::Playing) {
        // Atualize a lógica do jogo
        m_bird.update(deltaTime);  // Atualize a posição e a física do pássaro
        m_bamboo.update(deltaTime); // Atualize a posição dos obstáculos

        // Verifique colisões
        checkCollisions();

        // Verifique a condição de vitória (se o jogador ganhou)
        checkWinCondition();
    }
}


void Window::onPaint() {
    // Limpar a tela
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Renderizar o jogo, incluindo Bird, Bamboo e outros elementos
    bird.paintGL(m_viewMatrix, m_projectionMatrix);
    bamboo.paintGL(m_viewMatrix, m_projectionMatrix);
}

void Window::onPaintUI() {
    // Renderizar elementos da interface do usuário, como a pontuação
}

void Window::onResize(glm::ivec2 const &size) {
    // Redimensionar a janela e ajustar a matriz de projeção
}

void Window::onDestroy() {
    // Limpar recursos, como shaders, texturas, VAOs, VBOs, EBOs, etc.
}

void Window::restart() {
    // Reiniciar o jogo, redefinindo as posições, pontuação, estados, etc.
}

void Window::checkCollisions() {
    // Verificar colisões entre o Bird e os obstáculos
}

void Window::checkWinCondition() {
    // Verificar se o jogador venceu e atualizar o estado do jogo
}
