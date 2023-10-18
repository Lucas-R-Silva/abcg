#include "window.hpp"
#include <iostream>
#include <GL/glew.h> 

Bird bird;
Bamboo bamboo;
int m_score; 

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

GLuint loadShader(GLenum shaderType, const char *shaderSource) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, nullptr, infoLog.data());

        std::cerr << "Shader compilation error: " << infoLog.data() << std::endl;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog.data());

        std::cerr << "Program linking error: " << infoLog.data() << std::endl;

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void Window::onCreate() {
    // Configurar o programa OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Carregar shaders (suponha que você tenha funções para carregar shaders)
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "objects.vert");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "objects.frag");
    
    m_program = createProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Carregar texturas (suponha que você tenha funções para carregar texturas)

    // Inicializar objetos do jogo
    m_bird.initializeGL(m_program);
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
    // Renderizar elementos da interface do usuário
    if (m_gameData.m_state == State::Playing) {
        // Se o jogo estiver no estado "Playing", renderize a pontuação
        const auto scoreText = fmt::format("Score: {}", m_score);
        abcg::OpenGLWindow::getInstance().showMessageInUI(scoreText, 20, 20, 36, {1.0f, 1.0f, 1.0f, 1.0f});
    } else if (m_gameData.m_state == State::GameOver) {
        // Se o jogo estiver no estado "GameOver", renderize a mensagem de fim de jogo
        abcg::OpenGLWindow::getInstance().showMessageInUI("Game Over", 200, 300, 72, {1.0f, 0.0f, 0.0f, 1.0f});
    } else if (m_gameData.m_state == State::Win) {
        // Se o jogador ganhou, renderize uma mensagem de vitória
        abcg::OpenGLWindow::getInstance().showMessageInUI("You Win!", 200, 300, 72, {0.0f, 1.0f, 0.0f, 1.0f});
    }
}

void Window::onResize(glm::ivec2 const &size) {
    // Atualize o tamanho da viewport para coincidir com o novo tamanho da janela
    glViewport(0, 0, size.x, size.y);

    // Recalcule a matriz de projeção para manter a proporção da janela
    m_viewportSize = size;
    
    // Exemplo de cálculo de matriz de projeção com base na proporção da janela
    const float aspect = static_cast<float>(size.x) / size.y;
    m_projectionMatrix = glm::ortho(0.0f, 600.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    // Se você estiver usando uma câmera ortográfica, ajuste a matriz de projeção de acordo com suas necessidades
}

void Window::onDestroy() {
    // Exclua ou desvincule os recursos do OpenGL, como shaders, texturas, VAOs, VBOs, EBOs, etc.
    glDeleteProgram(m_program);
    m_bird.terminateGL();
    m_bamboo.terminateGL();
    
    // Certifique-se de adicionar a limpeza de outros recursos apropriados usados em seu jogo
}

void Window::restart() {
    // Redefina as variáveis e estados do jogo para os valores iniciais
    m_gameData.m_state = State::Playing;
    m_score = 0;
    
    // Redefina a posição e outros atributos do Bird
    m_bird.reset(); // Suponha que você tenha uma função reset() em Bird

    // Redefina as posições e outros atributos dos obstáculos (Bamboo)
    m_bamboo.reset(); // Suponha que você tenha uma função reset() em Bamboo

    // Outras reinicializações necessárias, como limpar listas de obstáculos, se aplicável

    // Reinicialize o timer de espera, se estiver sendo usado
    m_restartWaitTimer.restart();
}


void Window::checkCollisions() {
    // Obtém as coordenadas da caixa delimitadora do pássaro
    glm::vec2 birdMin = m_bird.getPosition() - m_bird.getSize() / 2.0f;
    glm::vec2 birdMax = m_bird.getPosition() + m_bird.getSize() / 2.0f;

    // Itera sobre os obstáculos (Bamboo) e verifica colisões
    for (const auto &bamboo : m_bamboo) {
        glm::vec2 bambooMin = bamboo.getPosition() - bamboo.getSize() / 2.0f;
        glm::vec2 bambooMax = bamboo.getPosition() + bamboo.getSize() / 2.0f;

        // Verifica se as caixas delimitadoras do pássaro e do obstáculo colidem
        if (birdMax.x >= bambooMin.x && birdMin.x <= bambooMax.x &&
            birdMax.y >= bambooMin.y && birdMin.y <= bambooMax.y) {
            // Colisão detectada: Faça algo, como definir o estado do jogo para GameOver
            m_gameData.m_state = State::GameOver;
            return; // Não é necessário verificar outras colisões
        }
    }
}
void Window::checkWinCondition() {
    // Verifique as condições de vitória do jogo
    if (m_score >= 30) {
        // O jogador venceu! Atualize o estado do jogo para Win
        m_gameData.m_state = State::Win;
    }
}