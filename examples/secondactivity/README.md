## Atividade 2 

Este projeto foi desenvolvido por:

* Aluno: Lucas Ribeiro da Silva     RA: 21006316
* Aluno: Guilherme Melo Ramos Amaro RA: 11087715

### Descrição

A movimentação de quatro cachorros dentro de uma cena. Os objetos (cachorros) são instânciados na cena em posições e ângulos diferentes.
Há um widget que é possível customizar a ação dentro da cena, como escala, altura e velocidade, além de pausar a movimentação dos objetos.
O observador da cena consegue se movimentar nela usando o teclado, foi usado como base o lookat de notas de aulas.

### Código

#### main.cpp

O arquivo main.cpp define a inicilização do programa

```
#include "window.hpp"

int main(int argc, char **argv) {
  try {
    abcg::Application app(argc, argv);

    Window window;
    window.setOpenGLSettings({.samples = 4});
    window.setWindowSettings({
        .width = 600,
        .height = 600,
        .title = "Segunda Atividade",
    });

    app.run(window);
  } catch (std::exception const &exception) {
    fmt::print(stderr, "{}\n", exception.what());
    return -1;
  }
  return 0;
}
```

### window.hpp

O arquivo window.hpp gerencia a aplicação armazendo variáveis e métodos.

```
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
```

### window.cpp

Arquivo responsável por configurar cena e objetos. Os métodos e linhas de comando estão comentados no decorrer do arquivo.

```
#include "window.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <unordered_map>

// Struct explicita de std::hash para Vertex
template <> struct std::hash<Vertex> {
  // Calcula o hash para um objeto Vertex.
  size_t operator()(Vertex const &vertex) const noexcept {
    // Calcula o hash da posição usando a especialização padrão de std::hash
    // para glm::vec3.
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    // Retorna o resultado do cálculo do hash.
    return h1;
  }
};

// Aqui captura-se eventos do teclado e mouse para movimentacao da camera
void Window::onEvent(SDL_Event const &event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_dollySpeed = 2.0;
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_dollySpeed = -2.0;
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_panSpeed = -2.0;
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_panSpeed = 2.0;
    if (event.key.keysym.sym == SDLK_q)
      m_truckSpeed = -2.0;
    if (event.key.keysym.sym == SDLK_e)
      m_truckSpeed = 2.0;
  }
  if (event.type == SDL_KEYUP) {
    if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_RIGHT ||
         event.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_q && m_truckSpeed < 0)
      m_truckSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_e && m_truckSpeed > 0)
      m_truckSpeed = 0.0f;
  }
}

/**
 Inicializa uma janela OpenGL, configurando a cor de limpeza,
 ativando o teste de profundidade, criando e carregando shaders,
 gerando buffers e objetos de array para um modelo 3D "dog",
 além de definir propriedades iniciais para múltiplas instâncias desses objetos.
 */
void Window::onCreate() {
  // Obtém o caminho dos ativos do aplicativo
  auto const &assetsPath{abcg::Application::getAssetsPath()};

  // Define a cor de limpeza para preto
  abcg::glClearColor(0, 0, 0, 1);

  // Habilita o teste de profundidade
  abcg::glEnable(GL_DEPTH_TEST);

  // Cria o programa OpenGL a partir dos shaders fornecidos
  m_program = abcg::createOpenGLProgram({{ .source = assetsPath + "lookat.vert", .stage = abcg::ShaderStage::Vertex}, {.source = assetsPath + "lookat.frag", .stage = abcg::ShaderStage::Fragment }});

  // Cria o plano (ground)
  m_ground.create(m_program);

  // Obtém as localizações das variáveis uniformes no shader
  m_viewMatrixLocation = abcg::glGetUniformLocation(m_program, "viewMatrix");
  m_projMatrixLocation = abcg::glGetUniformLocation(m_program, "projMatrix");
  m_modelMatrixLocation = abcg::glGetUniformLocation(m_program, "modelMatrix");
  m_colorLocation = abcg::glGetUniformLocation(m_program, "color");

  // Carrega o modelo do arquivo "dog.obj"
  loadModelFromFile(assetsPath + "dog.obj");

  // Gera o VBO (Vertex Buffer Object)
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER,
                     sizeof(m_vertices.at(0)) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Gera o EBO (Element Buffer Object)
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices.at(0)) * m_indices.size(),
                     m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Cria o VAO (Vertex Array Object)
  abcg::glGenVertexArrays(1, &m_VAO);

  // Vincula os atributos de vértice ao VAO atual
  abcg::glBindVertexArray(m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // Fim da vinculação ao VAO atual
  abcg::glBindVertexArray(0);
  /**
   *Abaixo vamos definir a escala e altura inicias dos cachorros
   */
  scale = 0.05f;
  height = 0.0f;

  /**
   *Aqui define-se a posição e o ângulo inicial de cada um dos 4 cachorros
   */
  dog[0].position.x = -3.0f;
  dog[0].position.y = height;
  dog[0].position.z = 3.0f;
  dog[0].angle = 90.0f;

  dog[1].position.x = 3.0f;
  dog[1].position.y = height;
  dog[1].position.z = 3.0f;
  dog[1].angle = 180.0f;

  dog[2].position.x = 3.0f;
  dog[2].position.y = height;
  dog[2].position.z = -3.0f;
  dog[2].angle = 270.0f;

  dog[3].position.x = -3.0f;
  dog[3].position.y = height;
  dog[3].position.z = -3.0f;
  dog[3].angle = 360.0f;
}

// Carrega um modelo 3D a partir de um arquivo dog.obj usando a biblioteca
// tinyobj.
void Window::loadModelFromFile(std::string_view path) {
  // Objeto para leitura do modelo 3D
  tinyobj::ObjReader reader;

  // Tenta fazer o parsing do modelo a partir do arquivo especificado
  if (!reader.ParseFromFile(path.data())) {
    // Trata erros de parsing e lança uma exceção em caso de falha
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  // Exibe warnings, se houver
  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  // Obtém os atributos e formas do modelo

  auto const &attributes{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};

  // Limpa os vetores de vértices e índices existentes
  m_vertices.clear();
  m_indices.clear();

  // Um mapa chave-valor com chave=Vertex e valor=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop sobre as formas do modelo
  for (auto const &shape : shapes) {
    // Loop sobre os índices da malha
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Acesso ao índice do vértice

      auto const index{shape.mesh.indices.at(offset)};

      // Posição do vértice
      auto const startIndex{3 * index.vertex_index};
      auto const vx{attributes.vertices.at(startIndex + 0)};
      auto const vy{attributes.vertices.at(startIndex + 1)};
      auto const vz{attributes.vertices.at(startIndex + 2)};

      // Cria um objeto Vertex com a posição do vértice
      Vertex const vertex{.position = {vx, vy, vz}};

      // Se o mapa não contém este vértice
      if (!hash.contains(vertex)) {
        // Adiciona este índice (tamanho de m_vertices)
        hash[vertex] = m_vertices.size();
        // Adiciona este vértice ao vetor de vértices
        m_vertices.push_back(vertex);
      }

      // Adiciona o índice ao vetor de índices
      m_indices.push_back(hash[vertex]);
    }
  }
}

// Função chamada para renderizar a cena na janela OpenGL
void Window::onPaint() {
  // Limpa o buffer de cor e o buffer de profundidade
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Define a área de visualização
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  // Usa o programa OpenGL especificado
  abcg::glUseProgram(m_program);

  // Configura as variáveis uniformes viewMatrix e projMatrix
  // Essas matrizes são usadas para cada objeto na cena
  abcg::glUniformMatrix4fv(m_viewMatrixLocation, 1, GL_FALSE,
                           &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(m_projMatrixLocation, 1, GL_FALSE,
                           &m_camera.getProjMatrix()[0][0]);

  // Vincula o Vertex Array Object (VAO) atual
  abcg::glBindVertexArray(m_VAO);

  /**
   * Chamamos a função drawDog para cada dos 4 cachorros passando a posição do
   * dog no array e sua cor rgb
   */
  drawDog(0, 2.0, 2.0, 2.0);
  drawDog(1, 0.0f, 2.0, 2.0);
  drawDog(2, 2.0, 0.0f, 2.0);
  drawDog(3, 2.0, 2.0, 0.0f);

  // Desvincula o Vertex Array Object (VAO)
  abcg::glBindVertexArray(0);

  // Desenha o plano (ground)
  m_ground.paint();

  // Desativa o programa OpenGL
  abcg::glUseProgram(0);
}

// Função para desenhar um modelo de cachorro na cena
void Window::drawDog(int i, float color_r, float color_g, float color_b) {
  // Criação da matriz de modelo para o cachorro
  glm::mat4 model{2.0};
  // Translação do cachorro para sua posição no plano, levando em consideração a
  // altura

  model = glm::translate(
      model, glm::vec3(dog[i].position.x, height, dog[i].position.z));
  // Rotação do cachorro em torno do eixo y
  model = glm::rotate(model, glm::radians(dog[i].angle), glm::vec3(0, 1, 0));

  // Escala do modelo do cachorro
  model = glm::scale(model, glm::vec3(scale));

  // Atualiza as variáveis uniformes nos shaders com os valores da matriz de
  // modelo e cor
  abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
  abcg::glUniform4f(m_colorLocation, color_r, color_g, color_b, 2.0);

  // Desenha o modelo do cachorro usando elementos (índices)
  abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT,
                       nullptr);
}

// Função para renderizar a interface do usuário (UI) na janela OpenGL
void Window::onPaintUI() {
  // Chama a função onPaintUI da classe base para manter funcionalidades padrão
  abcg::OpenGLWindow::onPaintUI();

  // Define o tamanho do widget na interface
  auto const widgetSize{ImVec2(218, 180)};

  // Define a posição da janela do widget
  ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x / 2, 0));
  // Define o tamanho da janela do widget
  ImGui::SetNextWindowSize(widgetSize);
  // Inicia a criação da janela do widget
  ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);

  {
    // Adiciona controles à janela do widget
    ImGui::RadioButton("start", &startGame, 1);
    ImGui::SameLine();
    ImGui::RadioButton("pause", &startGame, 0);
    ImGui::SliderFloat("Vel. Movimento", &MovementVelocity, 0.0f, 5.0f, "%.1f");
    ImGui::SliderFloat("Vel. Rotação", &RotationVelocity, 0.0f, 200.0f, "%.1f");
    ImGui::SliderFloat("Escala", &scale, 0.1f, 0.2f, "%.2f");
    ImGui::SliderFloat("Altura", &height, 0.0f, 2.0, "%.2f");
  }
  // Finaliza a criação da janela do widget
  ImGui::End();
}

// Função chamada para atualizar o estado da cena em cada quadro
void Window::onUpdate() {
  // Atualiza a posição de cada um dos 4 cachorros na cena
  updateDogPosition(0);
  updateDogPosition(1);
  updateDogPosition(2);
  updateDogPosition(3);
}

// Função para atualizar a posição de um cachorro e da câmera na cena
void Window::updateDogPosition(int i) {
  // Obtém o intervalo de tempo entre frames
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  // Verifica se o jogo está em execução
  if (startGame) {
    // Variáveis auxiliares para facilitar a compreensão dos condicionais
    float X = dog[i].position.x;
    float Z = dog[i].position.z;
    float A = dog[i].angle;

    // Rotacionando no Ponto A
    if (X <= -3 && Z >= 3 && A <= 90.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do Ponto A ao Ponto B
    else if (X <= 3 && Z >= 3 && A >= 90.0f) {
      X += deltaTime * MovementVelocity;
    }

    // Rotacionando no Ponto B
    if (X >= 3 && Z >= 3 && A <= 180.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do Ponto B ao Ponto C
    else if (X >= 3 && Z >= -3 && A >= 180.0f) {
      Z -= deltaTime * MovementVelocity;
    }

    // Rotacionando no Ponto C
    else if (X >= 3 && Z <= -3 && A <= 270.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do Ponto C ao Ponto D
    else if (X >= -3 && Z <= -3 && A >= 270.0f) {
      X -= deltaTime * MovementVelocity;
    }

    // Ao chegar no Ponto D definimos o ângulo = 0, pois 360° == 0°
    else if (X <= -3 && Z <= -3 && A >= 360.0f) {
      A = 0.0f;
    }

    // Rotacionando no Ponto D
    else if (X <= -3 && Z <= -3 && A <= 0.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do ponto D ao Ponto A
    else if (X <= -3 && Z <= 3 && A >= 0.0f) {
      Z += deltaTime * MovementVelocity;
    }

    // Atualizando posições e ângulos com os novos valores
    dog[i].position.x = X;
    dog[i].position.z = Z;
    dog[i].angle = A;
  }

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
}

// Função chamada quando a janela é redimensionada
void Window::onResize(glm::ivec2 const &size) {
  // Atualiza o tamanho da viewport com as novas dimensões
  m_viewportSize = size;
  // Recalcula a matriz de projeção da câmera para acomodar o novo tamanho
  m_camera.computeProjectionMatrix(size);
}

// Função chamada quando a janela está sendo destruída
void Window::onDestroy() {
  // Destroi o plano (ground)
  m_ground.destroy();

  // Deleta o programa OpenGL
  abcg::glDeleteProgram(m_program);
  // Deleta os buffers de elementos (EBO) e de vértices (VBO)
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  // Deleta o Vertex Array Object (VAO)
  abcg::glDeleteVertexArrays(1, &m_VAO);
}
```

Os demais arquivos foram uma importação do projeto lookat, sem nenhuma modificação relevante.


#### Página Web com Aplicação
https://lucas-r-silva.github.io/abcg/public/index.html