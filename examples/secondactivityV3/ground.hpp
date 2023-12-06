#ifndef GROUND_HPP_
#define GROUND_HPP_

#include "abcgOpenGL.hpp"

// Classe representando o objeto Ground (plano) na cena
class Ground {
public:
  // Função para criar os buffers e configurar atributos do objeto Ground
  void create(GLuint program);
  // Função para renderizar o objeto Ground na cena
  void paint();
  // Função para destruir os buffers do objeto Ground
  void destroy();

private:
  // Identificadores OpenGL para o Vertex Array Object (VAO) e Vertex Buffer
  // Object (VBO)
  GLuint m_VAO{};
  GLuint m_VBO{};
  // Localizações de variáveis uniformes nos shaders
  GLint m_modelMatrixLoc{};
  GLint m_colorLoc{};
};

#endif