#include "bird.hpp"

void Bird::createBirdGeometry() {
    // Defina os vértices do pássaro (quadrado)
    const std::vector<GLfloat> vertices = {
        -0.05f, -0.05f,  // Canto inferior esquerdo
         0.05f, -0.05f,  // Canto inferior direito
         0.05f,  0.05f,  // Canto superior direito
        -0.05f,  0.05f   // Canto superior esquerdo
    };

    // Defina as coordenadas de textura (pode ser um quadrado texturizado)
    const std::vector<GLfloat> textureCoords = {
        0.0f, 1.0f,  // Canto inferior esquerdo
        1.0f, 1.0f,  // Canto inferior direito
        1.0f, 0.0f,  // Canto superior direito
        0.0f, 0.0f   // Canto superior esquerdo
    };

    // Defina os índices para desenhar os triângulos do quadrado
    const std::vector<GLuint> indices = {
        0, 1, 2,  // Triângulo 1
        0, 2, 3   // Triângulo 2
    };

    // Crie e configure o Vertex Array Object (VAO)
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Crie e configure o Vertex Buffer Object (VBO) para as coordenadas dos vértices
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // Configure o atributo de posição dos vértices no shader
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    // Crie e configure o Vertex Buffer Object (VBO) para as coordenadas de textura
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(GLfloat), textureCoords.data(), GL_STATIC_DRAW);

    // Configure o atributo de textura no shader
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    // Crie e configure o Element Buffer Object (EBO) para os índices
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Desvincule o VAO para evitar modificações acidentais
    glBindVertexArray(0);
}


// Método de inicialização do Bird
void Bird::initializeGL(GLuint program) {
  m_program = program;
  createBirdGeometry();  // Implemente essa função para criar a geometria do pássaro
  // Carregue a textura do pássaro aqui (m_texture) usando OpenGL
}

// Método de renderização do Bird
void Bird::paintGL(const glm::mat4 &view, const glm::mat4 &projection) {
  glUseProgram(m_program);

  // Configure a posição, tamanho e rotação do pássaro no shader

// Configure a posição do pássaro
glUniform2fv(glGetUniformLocation(m_program, "birdPosition"), 1, &m_position.x);

// Configure o tamanho do pássaro
glUniform2fv(glGetUniformLocation(m_program, "birdSize"), 1, &m_size.x);

// Configure a rotação do pássaro
glUniform1f(glGetUniformLocation(m_program, "birdRotation"), m_rotation);



  // Configure o VAO, VBO, EBO, etc., para renderizar o pássaro
  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
  glUseProgram(0);
}

// Método de encerramento do Bird
void Bird::terminateGL() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_ebo);
  glDeleteVertexArrays(1, &m_vao);
  // Outras limpezas necessárias
}

// Método de atualização do Bird
void Bird::update(float deltaTime) {
  // Aplicar a gravidade ao pássaro (pode ser uma força para baixo constante)
  m_velocity.y += m_gravity * deltaTime;

  // Limitar a velocidade máxima do pássaro
  if (m_velocity.y > m_maxVelocity) {
    m_velocity.y = m_maxVelocity;
  }

  // Atualizar a posição do pássaro com base na velocidade
  m_position += m_velocity * deltaTime;

  // Certifique-se de lidar com colisões com o chão ou teto aqui e ajustar a posição do pássaro, se necessário.
}
// Método para fazer o Bird pular
void Bird::jump() {
  m_jump = true;
}

