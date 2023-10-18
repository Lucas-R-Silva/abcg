#include "bamboo.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Bamboos::create(GLuint program, int quantity) {
  destroy();

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;

  // Get location of uniforms in the program
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create bamboos
  m_Bamboos.clear();
  m_Bamboos.resize(quantity);

 for (auto &bamboo : m_Bamboos) {
    bamboo = makeBamboo();

    // Initialize the bamboo pairs at the top of the game area
    bamboo.m_translation = {m_randomDist(m_randomEngine), 1.2f};
  }
}

void Bamboos::paint() {
  abcg::glUseProgram(m_program);

  for (auto const &bamboo : m_Bamboos) {
    abcg::glBindVertexArray(bamboo.m_VAO);

    abcg::glUniform4fv(m_colorLoc, 1, &bamboo.m_color.r);
    abcg::glUniform1f(m_scaleLoc, bamboo.m_scale);
    abcg::glUniform1f(m_rotationLoc, bamboo.m_rotation);

    for (auto i : {-2, 0, 2}) {
      for (auto j : {-2, 0, 2}) {
        abcg::glUniform2f(m_translationLoc, bamboo.m_translation.x + j,
                          bamboo.m_translation.y + i);

        abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, bamboo.m_polygonSides + 2);
      }
    }

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}

void Bamboos::destroy() {
  for (auto &bamboo : m_Bamboos) {
    abcg::glDeleteBuffers(1, &bamboo.m_VBO);
    abcg::glDeleteVertexArrays(1, &bamboo.m_VAO);
  }
}

void Bamboos::update(const Bird &bird, float deltaTime) {
  for (auto &bamboo : m_Bamboos) {
    // Move bamboos downward
    bamboo.m_translation.y -= 0.2f * deltaTime;
    bamboo.m_rotation = glm::wrapAngle(
        bamboo.m_rotation + bamboo.m_angularVelocity * deltaTime);
    bamboo.m_translation += bamboo.m_velocity * deltaTime;

    // Wrap-around (reset at the top when off-screen)
    if (bamboo.m_translation.y < -1.2f) {
      bamboo.m_translation = {m_randomDist(m_randomEngine), 1.2f};
    }
  }
}

Bamboos::Bamboo Bamboos::makeBamboo(glm::vec2 translation, float scale) {
  Bamboo bamboo;

  auto &re{m_randomEngine}; // Shortcut

  // Retângulo
  bamboo.m_polygonSides = 4;

  // Get a random color (actually, a grayscale)
  std::uniform_real_distribution randomIntensity(0.5f, 1.0f);
  bamboo.m_color = glm::vec4(randomIntensity(re));

  bamboo.m_color.a = 1.0f;
  bamboo.m_rotation = 0.0f;
  bamboo.m_scale = scale;
  bamboo.m_translation = translation;

  // Get a random angular velocity
  bamboo.m_angularVelocity = m_randomDist(re);

  // Get a random direction
  glm::vec2 const direction{m_randomDist(re), m_randomDist(re)};
  bamboo.m_velocity = glm::normalize(direction) / 7.0f;

  // Criação dos vértices do retângulo
  std::vector<glm::vec2> positions{
      {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
  
  // Generate VBO
  abcg::glGenBuffers(1, &bamboo.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, bamboo.m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &bamboo.m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(bamboo.m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, bamboo.m_VBO);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  return bamboo;
}