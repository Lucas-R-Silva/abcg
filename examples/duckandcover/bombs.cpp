#include "bombs.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>


void Bombs::initializeGL(GLuint program, int quantity) {
  terminateGL();

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_rotationLoc = abcg::glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create bombs
  m_bombs.clear();
  m_bombs.resize(quantity);

  for (auto &bomb : m_bombs) {
    bomb = createBomb();

    // Make sure the bomb won't collide with the duck

    do {
      bomb.m_translation = {m_randomDist(m_randomEngine),
                                m_randomDist(m_randomEngine)+0.5f};
    } while (glm::length(bomb.m_translation) < 0.5f);
  }
}

void Bombs::paintGL() {
  abcg::glUseProgram(m_program);

  for (const auto &bomb : m_bombs) {
    abcg::glBindVertexArray(bomb.m_vao);

    abcg::glUniform4fv(m_colorLoc, 1, &bomb.m_color.r);
    abcg::glUniform1f(m_scaleLoc, bomb.m_scale);
    abcg::glUniform1f(m_rotationLoc, bomb.m_rotation);

    for (auto i : {-2, 0, 2}) {
      for (auto j : {-2, 0, 2}) {
        abcg::glUniform2f(m_translationLoc, bomb.m_translation.x + j,
                          bomb.m_translation.y + i);

        abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, bomb.m_polygonSides + 2);
      }
    }

    abcg::glBindVertexArray(0);
  }

  abcg::glUseProgram(0);
}

void Bombs::terminateGL() {
  for (auto bomb : m_bombs) {
    abcg::glDeleteBuffers(1, &bomb.m_vbo);
    abcg::glDeleteVertexArrays(1, &bomb.m_vao);
  }
}

void Bombs::update(const Duck &duck, float deltaTime) {
  auto &re{m_randomEngine};

  for (auto &bomb : m_bombs) {
    if (bomb.m_translation.y >= 0.99f) {
      bomb.m_translation.x = m_randomDist(re);
    }
      // glm::vec2 left{glm::vec2{1.0f, 0.0f}};
      //bomb.m_translation.x += 0.25f;
      //bomb.m_translation.y += 0.25f;
      bomb.m_translation.y -= (duck.m_velocity * deltaTime).y;
      float valor = (bomb.m_translation).x + (bomb.m_translation).y ;
      //  asteroid.m_translation.y -= valor ;
      bomb.m_translation.x -= 0.000001f;
      // bomb.m_translation.x = 0.00001f;
      // bomb.m_rotation = glm::wrapAngle(
      //     bomb.m_rotation + bomb.m_angularVelocity * deltaTime);
      bomb.m_translation.y += (bomb.m_velocity * deltaTime * 1.0f).y;
      // bomb.m_translation.x = 
      float valor2 = (bomb.m_velocity * deltaTime).x + (bomb.m_velocity * deltaTime).y;
      // bomb.m_translation.y += valor2;
      bomb.m_translation.x += 0.000001f;
      // Wrap-around
      if (bomb.m_translation.x < -1.0f) bomb.m_translation.y += 2.0f;
      if (bomb.m_translation.x > +1.0f) bomb.m_translation.y -= 2.0f;
      if (bomb.m_translation.y < -1.0f) bomb.m_translation.y += 2.0f;
      if (bomb.m_translation.y > +1.0f) bomb.m_translation.y -= 2.0f;
      // createAsteroid({asteroid.m_translation.x, 0.0f});
  }
}

Bombs::Bomb Bombs::createBomb(glm::vec2 translation,
                                              float scale) {
  Bomb bomb;

  auto &re{m_randomEngine};  // Shortcut

  // Randomly choose the number of sides
  std::uniform_int_distribution<int> randomSides(6, 20);
  bomb.m_polygonSides = randomSides(re);

  // Choose a random color (actually, a grayscale)
  std::uniform_real_distribution<float> randomIntensity(0.5f, 1.0f);
  bomb.m_color = glm::vec4(1) * randomIntensity(re);

  bomb.m_color.a = 1.0f;
  bomb.m_rotation = 0.0f;
  bomb.m_scale = scale;
  bomb.m_translation = translation;

  // Choose a random angular velocity
  bomb.m_angularVelocity = m_randomDist(re);

  // Choose a random direction
  glm::vec2 direction{10.0f, 0.0f};
  bomb.m_velocity = glm::normalize(direction) / 7.0f;

  // Create geometry
  std::vector<glm::vec2> positions(0);
  positions.emplace_back(0, 0);
  const auto step{M_PI * 2 / bomb.m_polygonSides};
  std::uniform_real_distribution<float> randomRadius(0.4f, 0.6f);
  for (const auto angle : iter::range(0.0, M_PI * 2, step)) {
    const auto radius{randomRadius(re)};
    positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Generate VBO
  abcg::glGenBuffers(1, &bomb.m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, bomb.m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &bomb.m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(bomb.m_vao);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, bomb.m_vbo);
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  return bomb;
}