#pragma once

#include <abcg.hpp>
#include <GL/glew.h>

class Bamboo {
public:
    void initializeGL(GLuint program);
    void paintGL(const glm::mat4 &model, GLuint texture);
    void terminateGL();

    void update(float deltaTime);
    void reset();  // Função para redefinir a posição do Bamboo

    const glm::vec2 &getPosition() const { return m_position; }
    const glm::vec2 &getSize() const { return m_size; }

private:
    GLuint m_program{};
    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};

    glm::vec2 m_position{0.0f, 0.0f};
    glm::vec2 m_velocity{-0.1f, 0.0f};  // Velocidade de movimento do Bamboo
    glm::vec2 m_size{0.1f, 0.5f};      // Tamanho do Bamboo

    void createBambooGeometry();
};

