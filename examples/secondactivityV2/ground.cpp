#include "ground.hpp"

// Função para criar o objeto Ground (plano) na cena
void Ground::create(GLuint program) {
  // Define as coordenadas dos vértices para formar um quadrado unitário no
  // plano xz
  std::array<glm::vec3, 4> vertices{{{-0.5f, 0.0f, +0.5f},
                                     {-0.5f, 0.0f, -0.5f},
                                     {+0.5f, 0.0f, +0.5f},
                                     {+0.5f, 0.0f, -0.5f}}};

  // Gera o Vertex Buffer Object (VBO) e preenche-o com os vértices
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Gera o Vertex Array Object (VAO) e configura os atributos dos vértices
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  auto const positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);

  // Salva as localizações das variáveis uniformes nos shaders
  m_modelMatrixLoc = abcg::glGetUniformLocation(program, "modelMatrix");
  m_colorLoc = abcg::glGetUniformLocation(program, "color");
}

// Função para renderizar o objeto Ground (plano) na cena
void Ground::paint() {
  // Associa o Vertex Array Object (VAO) atual
  abcg::glBindVertexArray(m_VAO);

  // Desenha uma grade de azulejos 2N+1 x 2N+1 no plano xz, centrada na origem
  auto const N{5};
  for (auto const z : iter::range(-N, N + 1)) {
    for (auto const x : iter::range(-N, N + 1)) {
      // Configura a matriz de modelo como uma matriz de translação
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 0.0f, z));
      abcg::glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, &model[0][0]);

      // Define a cor (padrão de tabuleiro de xadrez)
      auto const gray{(z + x) % 2 == 0 ? 1.0f : 0.5f};
      abcg::glUniform4f(m_colorLoc, gray, gray, gray, 1.0f);

      // Desenha um quadrado usando triângulos
      abcg::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
  }

  // Desassocia o Vertex Array Object (VAO) atual
  abcg::glBindVertexArray(0);
}

// Função para destruir os buffers do objeto Ground (plano)
void Ground::destroy() {
  // Deleta o Vertex Buffer Object (VBO) associado
  abcg::glDeleteBuffers(1, &m_VBO);
  // Deleta o Vertex Array Object (VAO) associado
  abcg::glDeleteVertexArrays(1, &m_VAO);
}