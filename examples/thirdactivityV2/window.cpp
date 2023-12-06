#include "window.hpp"

// Controla os eventos de mouse
void Window::onEvent(SDL_Event const &event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_MOUSEMOTION) {
    m_trackBallModel.mouseMove(mousePosition);
    m_trackBallLight.mouseMove(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (event.button.button == SDL_BUTTON_LEFT) {
      m_trackBallModel.mousePress(mousePosition);
    }
    if (event.button.button == SDL_BUTTON_RIGHT) {
      m_trackBallLight.mousePress(mousePosition);
    }
  }
  if (event.type == SDL_MOUSEBUTTONUP) {
    if (event.button.button == SDL_BUTTON_LEFT) {
      m_trackBallModel.mouseRelease(mousePosition);
    }
    if (event.button.button == SDL_BUTTON_RIGHT) {
      m_trackBallLight.mouseRelease(mousePosition);
    }
  }
  if (event.type == SDL_MOUSEWHEEL) {
    m_zoom += (event.wheel.y > 0 ? -1.0f : 1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 1.0f);
  }
}

// Executado durante a criação da janela, inicializa configurações e carrega
// recursos necessários.
void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  // Definir a cor de fundo da janela como preta
  abcg::glClearColor(0, 0, 0, 1);

  // Habilitar o teste de profundidade e o culling (remoção de faces ocultas)
  abcg::glEnable(GL_DEPTH_TEST);
  abcg::glEnable(GL_CULL_FACE);

  // Criar programas OpenGL para cada shader especificado nos nomes de shader
  for (auto const &name : m_shaderNames) {
    auto const path{assetsPath + "shaders/" + name};
    auto const program{abcg::createOpenGLProgram(
        {{.source = path + ".vert", .stage = abcg::ShaderStage::Vertex},
         {.source = path + ".frag", .stage = abcg::ShaderStage::Fragment}})};
    m_programs.push_back(program);
  }

  // Carregar o modelo padrão (UFO.obj) e definir o modo de mapeamento como
  // "From mesh"
  loadModel(assetsPath + "UFO.obj");
  m_mappingMode = 3;

  // Inicializar a trackball do modelo com um eixo e velocidade iniciais
  m_trackBallModel.setAxis(glm::normalize(glm::vec3(1, 1, 1)));
  m_trackBallModel.setVelocity(0.1f);
}

/**
 * Carrega um modelo a partir do caminho especificado e configura suas texturas
 * e VAO.
 *
 * Esta função realiza as seguintes etapas:
 *   1. Obtém o caminho do diretório de ativos.
 *   2. Destrói o modelo atualmente carregado.
 *   3. Carrega texturas difusas, normais e de modelo antigo para o modelo a partir de
 * caminhos específicos.
 *   4. Carrega o modelo Wavefront OBJ a partir do caminho especificado.
 *   5. Configura o Array de Objetos de Vértices (VAO) do modelo usando o
 * programa OpenGL atual.
 *   6. Define o número de triângulos a serem desenhados com base no modelo
 * carregado.
 *   7. Obtém as propriedades de material do modelo carregado.
 */
void Window::loadModel(std::string_view path) {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  m_model.destroy();

  m_model.loadDiffuseTexture(assetsPath + "maps/UFO_Glass_BaseColor.png");
  m_model.loadNormalTexture(assetsPath + "maps/UFO_Glass_Normal.png");
  m_model.loadAntigoTexture(assetsPath + "maps/UFO_Metal_BaseColor.png");
  m_model.loadObj(path);
  m_model.setupVAO(m_programs.at(m_currentProgramIndex));
  m_trianglesToDraw = m_model.getNumTriangles();

  m_Ka = m_model.getKa();
  m_Kd = m_model.getKd();
  m_Ks = m_model.getKs();
  m_shininess = m_model.getShininess();
}

/**
 * Função chamada para renderizar a cena na janela.
 *
 * Esta função realiza as seguintes etapas:
 *   1. Limpa os buffers de cor e profundidade.
 *   2. Define a viewport com base no tamanho da janela.
 *   3. Seleciona o programa OpenGL atual.
 *   4. Obtém a localização de variáveis uniformes no programa.
 *   5. Define variáveis uniformes que têm o mesmo valor para todos os modelos.
 *   6. Obtém a direção da luz rotacionada pela trackball da luz.
 *   7. Define as variáveis uniformes específicas do modelo atual.
 *   8. Calcula a matriz de visão e projeção e define variáveis uniformes
 * correspondentes.
 *   9. Chama a função para renderizar o modelo atual, especificando a
 * quantidade de triângulos a desenhar.
 *  10. Desativa o programa OpenGL atual.
 *
 * Esta função presume que algumas variáveis de membro (por exemplo,
 * m_viewMatrix, m_projMatrix, etc.) já estão configuradas.
 *
 */
void Window::onPaint() {
  // Limpar os buffers de cor e profundidade
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Definir a viewport com base no tamanho da janela
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  // Selecionar o programa OpenGL atual
  auto const program{m_programs.at(m_currentProgramIndex)};
  abcg::glUseProgram(program);

  // Obter a localização de variáveis uniformes no programa
  auto const viewMatrixLoc{abcg::glGetUniformLocation(program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(program, "projMatrix")};
  auto const modelMatrixLoc{abcg::glGetUniformLocation(program, "modelMatrix")};
  auto const normalMatrixLoc{
      abcg::glGetUniformLocation(program, "normalMatrix")};
  auto const lightDirLoc{
      abcg::glGetUniformLocation(program, "lightDirWorldSpace")};
  auto const shininessLoc{abcg::glGetUniformLocation(program, "shininess")};
  auto const IaLoc{abcg::glGetUniformLocation(program, "Ia")};
  auto const IdLoc{abcg::glGetUniformLocation(program, "Id")};
  auto const IsLoc{abcg::glGetUniformLocation(program, "Is")};
  auto const KaLoc{abcg::glGetUniformLocation(program, "Ka")};
  auto const KdLoc{abcg::glGetUniformLocation(program, "Kd")};
  auto const KsLoc{abcg::glGetUniformLocation(program, "Ks")};
  auto const diffuseTexLoc{abcg::glGetUniformLocation(program, "diffuseTex")};
  auto const normalTexLoc{abcg::glGetUniformLocation(program, "normalTex")};
  auto const nightTexLoc{abcg::glGetUniformLocation(program, "nightTex")};
  auto const mappingModeLoc{abcg::glGetUniformLocation(program, "mappingMode")};

  // Definir variáveis uniformes que têm o mesmo valor para todos os modelos
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(normalTexLoc, 1);
  abcg::glUniform1i(nightTexLoc, 2);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);

  // Obter a direção da luz rotacionada pela trackball da luz
  auto const lightDirRotated{m_trackBallLight.getRotation() * m_lightDir};
  abcg::glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  abcg::glUniform4fv(IaLoc, 1, &m_Ia.x);
  abcg::glUniform4fv(IdLoc, 1, &m_Id.x);
  abcg::glUniform4fv(IsLoc, 1, &m_Is.x);

  // Definir variáveis uniformes específicas do modelo atual
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  // Calcular a matriz de visão e projeção e definir variáveis uniformes
  // correspondentes
  auto const aspect{gsl::narrow<float>(m_viewportSize.x) /
                    gsl::narrow<float>(m_viewportSize.y)};

  m_projMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 5.0f);

  auto const modelViewMatrix{glm::mat3(m_viewMatrix * m_modelMatrix)};
  auto const normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

  abcg::glUniform4fv(KaLoc, 1, &m_Ka.x);
  abcg::glUniform4fv(KdLoc, 1, &m_Kd.x);
  abcg::glUniform4fv(KsLoc, 1, &m_Ks.x);
  abcg::glUniform1f(shininessLoc, m_shininess);

  // Chamar a função para renderizar o modelo atual, especificando a quantidade
  // de triângulos a desenhar
  m_model.render(m_trianglesToDraw);

  // Desativar o programa OpenGL atual
  abcg::glUseProgram(0);
}

/**
 * Função chamada para atualizar a cena em cada quadro.
 *
 * Esta função realiza as seguintes etapas:
 *   1. Atualiza a rotação do modelo ao redor do seu próprio eixo (horizontal)
 * usando a trackball.
 *   2. Atualiza a rotação do modelo ao redor do seu próprio eixo (vertical) com
 * base no tempo.
 *   3. Combina as matrizes de rotação horizontal e vertical.
 *   4. Atualiza o movimento elíptico do modelo.
 *   5. Combina as matrizes de rotação e translação.
 *   6. Define a matriz de visão.
 *
 * Esta função presume que algumas variáveis de membro (por exemplo,
 * m_modelMatrix, m_zoom, etc.) já estão configuradas.
 *
 */
void Window::onUpdate() {
  // Atualizar a rotação do modelo ao redor do seu próprio eixo (horizontal)
  // usando a trackball
  m_modelMatrix = m_trackBallModel.getRotation();

  // Atualizar a rotação do modelo ao redor do seu próprio eixo (vertical) com
  // base no tempo
  static auto verticalStartTime{std::chrono::high_resolution_clock::now()};
  auto verticalCurrentTime{std::chrono::high_resolution_clock::now()};
  float verticalTime{
      std::chrono::duration<float>(verticalCurrentTime - verticalStartTime)
          .count()};

  float verticalRotationAngle = 0.3f * verticalTime;
  glm::mat4 verticalRotationMatrix = glm::rotate(
      glm::mat4(1.0f), verticalRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

  // Combina as matrizes de rotação horizontal e vertical
  m_modelMatrix = verticalRotationMatrix * m_modelMatrix;

  // Atualizar o movimento elíptico do modelo
  static auto ellipticalStartTime{std::chrono::high_resolution_clock::now()};
  auto ellipticalCurrentTime{std::chrono::high_resolution_clock::now()};
  float ellipticalTime{
      std::chrono::duration<float>(ellipticalCurrentTime - ellipticalStartTime)
          .count()};

  float ellipticalX = 0.5f * cos(0.5f * ellipticalTime);
  float ellipticalY = 0.2f * sin(0.5f * ellipticalTime);
  glm::mat4 translationMatrix = glm::translate(
      glm::mat4(1.0f), glm::vec3(ellipticalX, ellipticalY, 0.0f));

  // Combina as matrizes de rotação e translação
  m_modelMatrix = translationMatrix * m_modelMatrix;

  // Define a matriz de visão
  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f + m_zoom),
                  glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

// Função chamada para renderizar a interface do usuário.
void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  // Criar widget da janela principal
  {
    auto widgetSize{ImVec2(222, 87)};

    if (!m_model.isUVMapped()) {
      // Adicionar espaço extra para texto estático
      widgetSize.y += 26;
    }

    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr,
                 ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration);

    // Slider será esticado horizontalmente
    ImGui::PushItemWidth(widgetSize.x - 16);
    ImGui::SliderInt(" ", &m_trianglesToDraw, 0, m_model.getNumTriangles(),
                     "%d triangles");
    ImGui::PopItemWidth();

    // Caixa de combinação para texturas
    {
      static std::size_t currentIndex{};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Textures", m_shaderNames.at(currentIndex))) {
        for (auto const index : iter::range(m_shaderNames.size())) {
          auto const isSelected{currentIndex == index};
          if (ImGui::Selectable(m_shaderNames.at(index), isSelected))
            currentIndex = index;
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      // Configurar o VAO se o programa de shader tiver mudado
      if (gsl::narrow<int>(currentIndex) != m_currentProgramIndex) {
        m_currentProgramIndex = gsl::narrow<int>(currentIndex);
        m_model.setupVAO(m_programs.at(m_currentProgramIndex));
      }
    }

    if (!m_model.isUVMapped()) {
      ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mesh has no UV coords.");
    }

    ImGui::End();
  }
}

// Função chamada quando a janela é redimensionada.
void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_trackBallModel.resizeViewport(size);
  m_trackBallLight.resizeViewport(size);
}

// Função chamada ao destruir a janela.
void Window::onDestroy() {
  m_model.destroy();
  for (auto const &program : m_programs) {
    abcg::glDeleteProgram(program);
  }
}