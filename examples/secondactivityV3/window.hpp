#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"

#include "camera.hpp"
#include "ground.hpp"

// Definição da estrutura Vertex representando um vértice em um modelo 3D
struct Vertex {
  // Posição tridimensional do vértice
  glm::vec3 position;

  // Sobrecarga do operador de igualdade para permitir comparações entre
  // vértices
  friend bool operator==(Vertex const &, Vertex const &) = default;
};

// Classe herdando de abcg::OpenGLWindow representando a janela principal da
// aplicação
class Window : public abcg::OpenGLWindow {
protected:
  // Função chamada em resposta a eventos SDL
  void onEvent(SDL_Event const &event) override;

  // Função chamada na inicialização da aplicação
  void onCreate() override;

  // Função chamada para renderizar a cena
  void onPaint() override;

  // Função chamada para renderizar a interface do usuário (UI)
  void onPaintUI() override;

  // Função chamada quando a janela é redimensionada
  void onResize(glm::ivec2 const &size) override;

  // Função chamada quando a janela está sendo destruída
  void onDestroy() override;

  // Função chamada para atualizar o estado da cena em cada quadro
  void onUpdate() override;

private:
  // Tamanho da viewport
  glm::ivec2 m_viewportSize{};

  // Identificadores OpenGL para VAO, VBO, EBO e programa
  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};
  GLuint m_program{};

  // Localizações de variáveis uniformes nos shaders
  GLint m_viewMatrixLocation{};
  GLint m_projMatrixLocation{};
  GLint m_modelMatrixLocation{};
  GLint m_colorLocation{};

  // Instância da classe Camera para controle da visão
  Camera m_camera;

  // Variáveis que controlam a velocidade de movimento da câmera
  float m_dollySpeed{};
  float m_truckSpeed{};
  float m_panSpeed{};

  // Estrutura que define as propriedades de um cachorro
  struct Dog {
    glm::vec3 position{};
    float angle{};
  };

  // Variáveis que controlam a escala e altura dos cachorro
  float scale{};
  float height{};

  // Criando o array de cachorros
  std::array<Dog, 4> dog;

  // Variável que indica se o jogo está pausado ou em execução
  int startGame{0};

  // Variáveis que controlam a velocidade de movimento e rotação dos cachorros

  float MovementVelocity{0.5f};
  float RotationVelocity{50.0f};

  // Objeto que representa o chão na cena
  Ground m_ground;

  // Vetores de vértices e índices para o modelo 3D
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  // Função para carregar um modelo 3D a partir de um arquivo
  void loadModelFromFile(std::string_view path);

  // Função para desenhar um modelo de cachorro na cena
  void drawDog(int i, float color_r, float color_g, float color_b);
  
  // Função para atualizar a posição de um cachorro na cena
  void updateDogPosition(int i);
};

#endif