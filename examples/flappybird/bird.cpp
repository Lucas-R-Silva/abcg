#include "bird.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/constants.hpp>

void Bird::create(GLuint program) {
  destroy();

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Reset Bird attributes
  m_rotation = 0.0f;
  m_translation = glm::vec2(0);
  m_velocity = glm::vec2(0);

  // Define as coordenadas dos vértices do pássaro (simétrico)
  std::array positions{
    // Corpo do pássaro
    glm::vec2{0.0f, 0.0f},
    glm::vec2{0.1f, -0.2f},
    glm::vec2{0.15f, -0.3f},
    glm::vec2{0.1f, -0.4f},
    glm::vec2{0.0f, -0.5f},
    glm::vec2{-0.1f, -0.4f},
    glm::vec2{-0.15f, -0.3f},
    glm::vec2{-0.1f, -0.2f},

    // Asas
    glm::vec2{0.1f, -0.2f},
    glm::vec2{0.2f, -0.1f},
    glm::vec2{0.1f, 0.0f},
    glm::vec2{0.2f, 0.1f},
    glm::vec2{0.1f, 0.2f},

    // Cabeça
    glm::vec2{0.0f, 0.3f},
};

// Não é necessário normalizar as coordenadas no caso do pássaro

std::array const indices{
    // Corpo
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 5,
    0, 5, 6,
    0, 6, 7,

    // Asas
    8, 9, 10,
    8, 10, 11,

    // Cabeça
    12, 1, 2,
};

  // clang-format on

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void Bird::paint(const GameData &gameData) {
  if (gameData.m_state != State::Playing)
    return;

  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_VAO);

  abcg::glUniform1f(m_scaleLoc, m_scale);
  abcg::glUniform1f(m_rotationLoc, m_rotation);
  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);

  // Restart thruster blink timer every 100 ms
  if (m_trailBlinkTimer.elapsed() > 100.0 / 1000.0)
    m_trailBlinkTimer.restart();

  

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);
  abcg::glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}


void Bird::destroy() {
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}
void Bird::update(GameData const &gameData, float deltaTime) {
  if (gameData.m_state == State::Playing) {
    // Se a tecla "Up" estiver pressionada, aplique uma força de impulso para cima.
    if (gameData.m_input[gsl::narrow<size_t>(Input::Up)]) {
      m_velocity.y = 0.5f; // Ajuste a força do impulso conforme necessário.
    }

    // Aplique a força da gravidade para puxar o pássaro para baixo.
    m_velocity.y -= 0.2f * deltaTime; // Ajuste o valor da gravidade conforme necessário.

    // Atualize a posição vertical com base na velocidade.
    m_translation.y += m_velocity.y * deltaTime;

    // Adicione a lógica para limitar a altura máxima que o pássaro pode atingir.
    // Isso impede que o pássaro suba infinitamente.
    const float maxHeight = 1.0f; // Ajuste a altura máxima conforme necessário.
    if (m_translation.y > maxHeight) {
      m_translation.y = maxHeight;
      m_velocity.y = 0.0f; // Reset da velocidade para evitar que o pássaro continue subindo.
    }

    // Adicione a lógica para limitar a altura mínima que o pássaro pode atingir.
    const float minHeight = -1.0f; // Ajuste a altura mínima conforme necessário.
    if (m_translation.y < minHeight) {
      m_translation.y = minHeight;
      m_velocity.y = 0.0f; // Reset da velocidade para evitar que o pássaro continue caindo.
    }
  }
}
