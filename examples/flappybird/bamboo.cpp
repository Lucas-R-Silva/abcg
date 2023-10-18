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

  float spacing = 0.4f; // Espaçamento entre os pares de bambu

  for (auto &bamboo : m_Bamboos) {
    bamboo = makeBamboo();

    // Inicialize os pares de bambu na parte superior da área de jogo
    bamboo.m_translation = {m_randomDist(m_randomEngine), 1.2f};

    // Crie um segundo bamboo para formar um par alinhado
    Bamboo secondBamboo = makeBamboo();
    secondBamboo.m_translation = {bamboo.m_translation.x + spacing, 1.2f};

    m_Bamboos.push_back(secondBamboo);
  }
}

void Bamboos::paint() {
  abcg::glUseProgram(m_program);

  for (const auto &bamboo : m_Bamboos) {
    abcg::glBindVertexArray(bamboo.m_VAO);

    abcg::glUniform4fv(m_colorLoc, 1, &bamboo.m_color.r);
    abcg::glUniform1f(m_scaleLoc, bamboo.m_scale);
    abcg::glUniform1f(m_rotationLoc, bamboo.m_rotation);

    for (auto j : {-2, 0, 2}) {
      for (auto k : {-2, 0, 2}) {
        abcg::glUniform2f(m_translationLoc, bamboo.m_translation.x + k,
                          bamboo.m_translation.y + j);

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
    // Move os retângulos para a esquerda
    bamboo.m_translation.x -= 0.2f * deltaTime;
    bamboo.m_rotation = glm::wrapAngle(
        bamboo.m_rotation + bamboo.m_angularVelocity * deltaTime);

    // Wrap-around (reset na direita quando estiver fora da tela à esquerda)
    if (bamboo.m_translation.x < -1.2f) {
      bamboo.m_translation = {1.2f, m_randomDist(m_randomEngine)};
    }
  }
}

Bamboos::Bamboo Bamboos::makeBamboo(glm::vec2 translation, float scale) {
  Bamboo bamboo;

  auto &re{m_randomEngine}; // Shortcut

  // Retângulo
  bamboo.m_polygonSides = 4;

  // Defina a cor verde
  bamboo.m_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // Cor verde

  bamboo.m_rotation = 0.0f;
  bamboo.m_scale = scale;

  // Inicie o retângulo no lado direito da tela (x = 1.0) e vá até o lado esquerdo
  float rectHeight = 2.0f; // Altura do retângulo
  std::vector<glm::vec2> positions{
      {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -rectHeight}, {1.0f, -rectHeight}};

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

  // Defina a posição de translação recebida ou use a posição padrão
  bamboo.m_translation = translation;

  return bamboo;
}
