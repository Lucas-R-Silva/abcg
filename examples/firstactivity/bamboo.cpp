#include "bamboo.hpp"

#include <GL/glew.h>

void Bamboo::initializeGL(GLuint program) {
    m_program = program;
    createBambooGeometry();
}

void Bamboo::paintGL(const glm::mat4 &model, GLuint texture) {
    glUseProgram(m_program);

    glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_2D, texture);

    glUniformMatrix4fv(glGetUniformLocation(m_program, "model"), 1, GL_FALSE, &model[0][0]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);

    glUseProgram(0);
}

void Bamboo::terminateGL() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void Bamboo::update(float deltaTime) {
    // Atualize a posição do Bamboo com base na velocidade
    m_position += m_velocity * deltaTime;

    // Verifique se o Bamboo saiu da tela e, se sim, redefina sua posição
    if (m_position.x < -1.0f) {
        reset();
    }
}

void Bamboo::createBambooGeometry() {
    // Implemente a criação da geometria do Bamboo aqui, incluindo VAO, VBO, EBO, vértices, índices, etc.
    // Certifique-se de carregar texturas, shaders e outros recursos aqui

    // Exemplo de criação de VAO, VBO e EBO para um retângulo
    const std::array<GLfloat, 12> vertices{
        -m_size.x / 2, -m_size.y / 2, 0.0f,
        m_size.x / 2, -m_size.y / 2, 0.0f,
        m_size.x / 2, m_size.y / 2, 0.0f,
        -m_size.x / 2, m_size.y / 2, 0.0f,
    };

    const std::array<GLuint, 6> indices{
        0, 1, 2,
        0, 2, 3,
    };

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Configure os atributos de vértice e outros detalhes, como shaders, texturas, etc.

    glBindVertexArray(0);
}

void Bamboo::reset() {
    // Redefina a posição do Bamboo para a posição inicial (fora da tela)
    m_position = glm::vec2(1.2f, 0.0f);
}

