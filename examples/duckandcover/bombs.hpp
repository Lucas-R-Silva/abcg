#ifndef BOMBS_HPP_
#define BOMBS_HPP_

#include <list>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"
#include "duck.hpp"

class OpenGLWindow;

class Bombs {
 public:
  void initializeGL(GLuint program, int quantity);
  void paintGL();
  void terminateGL();

  void update(const Duck &duck, float deltaTime);

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_colorLoc{};
  GLint m_rotationLoc{};
  GLint m_translationLoc{};
  GLint m_scaleLoc{};

  struct Bomb {
    GLuint m_vao{};
    GLuint m_vbo{};

    float m_angularVelocity{};
    glm::vec4 m_color{1};
    bool m_hit{false};
    int m_polygonSides{};
    float m_rotation{};
    float m_scale{};
    glm::vec2 m_translation{glm::vec2(0)};
    glm::vec2 m_velocity{glm::vec2(0)};
  };

  std::list<Bomb> m_bombs;

  std::default_random_engine m_randomEngine;
  std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};

  Bombs::Bomb createBomb(glm::vec2 translation = glm::vec2(0),
                                     float scale = 0.25f);
};

#endif