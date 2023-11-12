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
      m_dollySpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_q)
      m_truckSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_e)
      m_truckSpeed = 1.0f;
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
  m_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "lookat.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "lookat.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

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

  // Configuração inicial de escala e altura dos cachorros
  scale = 0.05f;
  height = 0.0f;

  // Inicialização da posição e ângulo inicial dos 4 cachorros
  dog[0].position.x = -2.0f;
  dog[0].position.y = height;
  dog[0].position.z = 2.0f;
  dog[0].angle = 180.0f;

  dog[1].position.x = 2.0f;
  dog[1].position.y = height;
  dog[1].position.z = 2.0f;
  dog[1].angle = 270.0f;

  dog[2].position.x = 2.0f;
  dog[2].position.y = height;
  dog[2].position.z = -2.0f;
  dog[2].angle = 360.0f;

  dog[3].position.x = -2.0f;
  dog[3].position.y = height;
  dog[3].position.z = -2.0f;
  dog[3].angle = 90.0f;
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
  drawDog(0, 1.0f, 1.0f, 1.0f); // Cachorro branco
  drawDog(1, 0.0f, 1.0f, 1.0f); // Cachorro ciano
  drawDog(2, 1.0f, 0.0f, 1.0f); // Cachorro magenta
  drawDog(3, 1.0f, 1.0f, 0.0f); // Cachorro amarelo

  // Desvincula o Vertex Array Object (VAO)
  abcg::glBindVertexArray(0);

  // Desenha o plano (ground)
  m_ground.paint();

  // Desativa o programa OpenGL
  abcg::glUseProgram(0);
}

// Função para desenhar um modelo de cachorro na cena
void Window::drawDog(int i, float color_r, float color_g, float color_b) {
  // Matriz de modelo inicializada como uma matriz de identidade
  glm::mat4 model{1.0f};
  // Aplica as transformações de translação, rotação e escala ao modelo
  model = glm::translate(
      model, glm::vec3(dog[i].position.x, height, dog[i].position.z));
  model = glm::rotate(model, glm::radians(dog[i].angle), glm::vec3(0, 1, 0));
  model = glm::scale(model, glm::vec3(scale));

  // Define a matriz de modelo uniforme no shader
  abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
  // Define a cor uniforme no shader
  abcg::glUniform4f(m_colorLocation, color_r, color_g, color_b, 1.0f);
  // Desenha os elementos do modelo usando os índices
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
  ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - widgetSize.x - 5, 5));
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
    ImGui::SliderFloat("Altura", &height, 0.0f, 1.0f, "%.2f");
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
    if (X <= -2 && Z >= 2 && A <= 180.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do Ponto A ao Ponto B
    else if (X <= 2 && Z >= 2 && A >= 180.0f) {
      X += deltaTime * MovementVelocity;
    }

    // Rotacionando no Ponto B
    if (X >= 2 && Z >= 2 && A <= 270.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do Ponto B ao Ponto C
    else if (X >= 2 && Z >= -2 && A >= 270.0f) {
      Z -= deltaTime * MovementVelocity;
    }

    // Rotacionando no Ponto C
    else if (X >= 2 && Z <= -2 && A <= 360.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do Ponto C ao Ponto D
    else if (X >= -2 && Z <= -2 && A >= 360) {
      X -= deltaTime * MovementVelocity;
    }

    // Ao chegar no Ponto D definimos o ângulo = 0, pois 360° == 0°
    else if (X <= -2 && Z <= -2 && A >= 360) {
      A = 0.0f;
    }

    // Rotacionando no Ponto D
    else if (X <= -2 && Z <= -2 && A <= 90.0f) {
      A += deltaTime * RotationVelocity;
    }

    // Movimentando do ponto D ao Ponto A
    else if (X <= -2 && Z <= 2 && A >= 90.0f) {
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