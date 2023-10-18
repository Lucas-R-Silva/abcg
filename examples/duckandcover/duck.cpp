#include "duck.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <unistd.h>

void Duck::initializeGL(GLuint program) {
  terminateGL();

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  m_rotation = 0.0f;
  m_translation = glm::vec2(0);
  m_positionx = 0.0f;
  m_positiony = 0.0f;
  m_velocity = glm::vec2(0);
  
 // clang-format off
  std::array<glm::vec2, 24> positions{
      // Pato cabeca
    glm::vec2{+6.0f, +12.0f}, 
	  glm::vec2{+10.0f, +8.0f},
    glm::vec2{+6.0f, +4.0f}, 
	  glm::vec2{+2.0f, +8.0f},

      // Bico
	  glm::vec2{+10.0f, +8.0f},
	  glm::vec2{+14.0f, +4.0f},

	  // Pescoco
	  glm::vec2{+8.0f, +2.0f},
	  glm::vec2{+8.0f, -4.0f}, 
	  glm::vec2{+2.0f, +2.0f},

	  // Peito
	  glm::vec2{+2.0f, -10.0f},

	  // Asa
	  glm::vec2{-10.0f, -10.0f},

	  // Rabo
	  glm::vec2{-6.0f, -6.0f},
	  glm::vec2{-14.0f, +2.0f},

	  // Pata
	  glm::vec2{-2.0f, -10.0f},
	  glm::vec2{+2.0f, -12.0f}, 
	  glm::vec2{-6.0f, -12.0f},  };

  // Normalize
  for (auto &position : positions) {
    position /= glm::vec2{15.5f, 15.5f};
  }

  const std::array indices{0, 1, 2,
                           0, 2, 3,
                           2, 4, 5,
                           3, 6, 8,
                           6, 7, 8,
                           7, 8, 9,
                           8, 9, 10,
                           8, 11, 12,
                           13, 14, 15,
                           10, 11, 12,
                           10, 12, 13,
                           14, 15, 16,
                           14, 16, 17};

  // Generate VBO
  abcg::glGenBuffers(1, &m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_ebo);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void Duck::paintGL(const GameData &gameData) {
  if (gameData.m_state != State::Playing) return;

  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_vao);

  abcg::glUniform1f(m_scaleLoc, m_scale);
  abcg::glUniform1f(m_rotationLoc, m_rotation);
  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);

  // Restart thruster blink timer every 100 ms
  if (m_trailBlinkTimer.elapsed() > 100.0 / 1000.0) m_trailBlinkTimer.restart();

  // if (gameData.m_input[static_cast<size_t>(Input::JumpUp)]) {
  //   // Show thruster trail during 50 ms
  //   if (m_trailBlinkTimer.elapsed() < 50.0 / 1000.0) {
  //     abcg::glEnable(GL_BLEND);
  //     abcg::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //     // 50% transparent
  //     abcg::glUniform4f(m_colorLoc, 1, 1, 1, 0.5f);

  //     abcg::glDrawElements(GL_TRIANGLES, 14 * 3, GL_UNSIGNED_INT, nullptr);

  //     abcg::glDisable(GL_BLEND);
  //   }
  // }

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);
  abcg::glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Duck::terminateGL() {
  abcg::glDeleteBuffers(1, &m_vbo);
  abcg::glDeleteBuffers(1, &m_ebo);
  abcg::glDeleteVertexArrays(1, &m_vao);
}

void Duck::update(const GameData &gameData, float deltaTime) {
  // Rotate
  glm::vec2 forward = glm::rotate(glm::vec2{0.0f, 1.0f}, m_rotation);
  if (gameData.m_input[static_cast<size_t>(Input::Left)] &&
      gameData.m_state == State::Playing) {
    if (m_positionx > -0.5f)
      m_positionx -= 0.01f;
    m_translation = glm::vec2(m_positionx,m_positiony);
  }
  if (gameData.m_input[static_cast<size_t>(Input::Right)] &&
      gameData.m_state == State::Playing) {
    if (m_positionx < 0.5f)
      m_positionx += 0.01f;
    m_translation = glm::vec2(m_positionx,m_positiony);
  }

  // Apply thrust
  if (gameData.m_input[static_cast<size_t>(Input::JumpUp)] &&
      gameData.m_state == State::Playing) {
      if (m_positiony < 0.5f)
        m_positiony += 0.01f;
      m_translation = glm::vec2(m_positionx, m_positiony);
    } else if (gameData.m_input[static_cast<size_t>(Input::JumpDown)] &&
      gameData.m_state == State::Playing) {
      if (m_positiony > -0.5f)
        m_positiony -= 0.01f;
      m_translation = glm::vec2(m_positionx, m_positiony);
    }

     m_velocity += 0.25f * deltaTime;
}