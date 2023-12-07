## Atividade 3 -  Lost in the space

Este projeto foi desenvolvido por:

* Aluno: Lucas Ribeiro da Silva     RA: 21006316
* Aluno: Guilherme Melo Ramos Amaro RA: 11087715

### Descrição

O programa apresenta uma nave girando em torno do seu próprio eixo, com velocidade variável ao longo do tempo, e permite a manipulação da posição da câmera pelo observador
O usuário pode controlar a quantidade de triângulos preenchidos no modelo, escolher texturas e realizar rotação por meio do mouse.
Para implementar esses recursos, foram aplicados conceitos discutidos em sala de aula, como a utilização de uma trackball.

### Código

#### window.cpp

Função onUpdate()

Essa função atualiza a cena, ajustando a rotação do modelo e aplicando movimento elíptico.
Utilizando a trackball para atualizar a rotação horizontal do modelo e o tempo para controlar a rotação vertical. Além disso, aplica um movimento elíptico ao modelo, combinando as transformações de rotação e translação.

Passo a passo da descrição acima:

    1. Atualiza a rotação do modelo ao redor do seu próprio eixo (horizontal) usando a trackball.
    2. Atualiza a rotação do modelo ao redor do seu próprio eixo (vertical) com base no tempo.
    3. Combina as matrizes de rotação horizontal e vertical.
    4. Atualiza o movimento elíptico do modelo.
    5. Combina as matrizes de rotação e translação.
    6. Define a matriz de visão.
 
  Esta função presume que algumas variáveis de membro (por exemplo, m_modelMatrix, m_zoom, etc.) já estão configuradas.
 
```
void Window::onUpdate() {
  // Atualizar a rotação do modelo ao redor do seu próprio eixo (horizontal) usando a trackball
  m_modelMatrix = m_trackBallModel.getRotation();

  // Atualizar a rotação do modelo ao redor do seu próprio eixo (vertical) com base no tempo
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
```

#### model.cpp

Função loadNormalTexture(std::string_view path), loadDiffuseTexture(std::string_view path) e loadAntigoTexture(std::string_view path)

Carregam texturas de diferentes tipos no modelo, excluindo a atual carregada no estado do programa.

Passo a passo da descrição:
    1. Verifica se o arquivo da textura normal no caminho especificado existe.
    2. Deleta a textura normal atualmente associada ao modelo.
    3. Carrega a nova textura normal usando a função `abcg::loadOpenGLTexture`.
 
 Se o arquivo não existir, a função retorna sem fazer alterações.
 
```
void Model::loadNormalTexture(std::string_view path) {
  // Verificar se o arquivo da textura normal no caminho especificado existe
  if (!std::filesystem::exists(path))
    return;

  // Deletar a textura normal atualmente associada ao modelo
  abcg::glDeleteTextures(1, &m_normalTexture);

  // Carregar a nova textura normal usando a função `abcg::loadOpenGLTexture`
  m_normalTexture = abcg::loadOpenGLTexture({.path = path});
}
```


#### Página Web com Aplicação

https://lucas-r-silva.github.io/abcg/public/index.html
