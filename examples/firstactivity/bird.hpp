#pragma once

#include <abcg.hpp>
#include <GL/glew.h>

class Bird {
public:
  void initializeGL(GLuint program);
  void paintGL(const glm::mat4 &view, const glm::mat4 &projection);
  void terminateGL();

  void update(float deltaTime);
  void jump();
  void reset();

private:
  GLuint m_program{};
  GLuint m_vao{};
  GLuint m_vbo{};
  GLuint m_ebo{};
  GLuint m_texture{};

  glm::vec2 m_position{0.0f, 0.0f};
  glm::vec2 m_velocity{0.0f, 0.0f};
  glm::vec2 m_size{0.1f, 0.1f};

  float m_rotation{0.0f};
  float m_gravity{0.005f};
  float m_jumpSpeed{0.1f};
  float m_maxVelocity{0.15f};

  bool m_jump{false};

  void createBirdGeometry();
};

