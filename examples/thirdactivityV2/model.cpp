#include "model.hpp"

#include <filesystem>
#include <unordered_map>

// Struct explicita de std::hash para Vertex
template <> struct std::hash<Vertex> {
  // Calcula o hash para um objeto Vertex.
  size_t operator()(Vertex const &vertex) const noexcept {
    // Calcula o hash da posição usando a especialização padrão de std::hash
    // para glm::vec3.
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    auto const h2{std::hash<glm::vec3>()(vertex.normal)};
    auto const h3{std::hash<glm::vec2>()(vertex.texCoord)};
    // Retorna o resultado do cálculo do hash.
    return abcg::hashCombine(h1, h2, h3);
  }
};

/**
 * Calcula as normais dos vértices do modelo com base nas normais das faces.
 *
 * Esta função computa as normais dos vértices do modelo com base nas normais
 * das faces. Ela realiza os seguintes passos:
 *   1. Limpa as normais dos vértices anteriores, definindo-as como (0.0f, 0.0f,
 * 0.0f).
 *   2. Calcula as normais das faces e as acumula nos vértices correspondentes.
 *   3. Normaliza as normais dos vértices para garantir que tenham comprimento
 * unitário.
 *
 * Esta função assume que as informações dos vértices e índices do modelo já
 * foram carregadas e estão presentes nos membros de dados 'm_vertices' e
 * 'm_indices'.
 */
void Model::computeNormals() {
  // Limpar normais dos vértices anteriores
  for (auto &vertex : m_vertices) {
    vertex.normal = glm::vec3(0.0f);
  }

  // Calcular normais das faces e acumular nos vértices correspondentes
  for (auto const offset : iter::range<size_t>(0UL, m_indices.size(), 3UL)) {
    // Obter vértices da face
    auto &a{m_vertices.at(m_indices.at(offset + 0))};
    auto &b{m_vertices.at(m_indices.at(offset + 1))};
    auto &c{m_vertices.at(m_indices.at(offset + 2))};

    // Calcular normal da face usando o produto vetorial das arestas
    auto const edge1{b.position - a.position};
    auto const edge2{c.position - b.position};
    auto const normal{glm::cross(edge1, edge2)};

    // Acumular a normal nos vértices
    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  // Normalizar as normais dos vértices
  for (auto &vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  // Indica que as normais foram calculadas e estão disponíveis
  m_hasNormals = true;
}

/**
 * Calcula tangentes para os vértices do modelo com base nas coordenadas de
 * textura.
 *
 * Esta função calcula as tangentes dos vértices do modelo com base nas
 * coordenadas de textura para suportar o mapeamento de texturas normais. Ela
 * realiza os seguintes passos:
 *   1. Calcula tangentes e bitangentes para cada face do modelo.
 *   2. Acumula as tangentes nas informações de vértices correspondentes.
 *   3. Ortogonaliza as tangentes em relação às normais dos vértices.
 *   4. Calcula a orientação (handedness) da base ortogonalizada.
 *
 * Esta função assume que as informações dos vértices e índices do modelo já
 * foram carregadas e estão presentes nos membros de dados 'm_vertices' e
 * 'm_indices'. Além disso, espera-se que as coordenadas de textura (texCoord)
 * estejam presentes nos vértices do modelo.
 */
void Model::computeTangents() {
  // Reservar espaço para bitangentes
  std::vector bitangents(m_vertices.size(), glm::vec3(0));

  // Calcular tangentes e bitangentes para cada face
  for (auto const offset : iter::range<size_t>(0UL, m_indices.size(), 3UL)) {
    // Obter índices da face
    auto const i1{m_indices.at(offset + 0)};
    auto const i2{m_indices.at(offset + 1)};
    auto const i3{m_indices.at(offset + 2)};

    // Obter vértices da face
    auto &v1{m_vertices.at(i1)};
    auto &v2{m_vertices.at(i2)};
    auto &v3{m_vertices.at(i3)};

    // Calcular vetores de aresta e delta de coordenadas de textura
    auto const e1{v2.position - v1.position};
    auto const e2{v3.position - v1.position};
    auto const delta1{v2.texCoord - v1.texCoord};
    auto const delta2{v3.texCoord - v1.texCoord};

    // Construir matriz de transformação para calcular tangente
    glm::mat2 M;
    M[0][0] = delta2.t;
    M[0][1] = -delta1.t;
    M[1][0] = -delta2.s;
    M[1][1] = delta1.s;
    M *= (1.0f / (delta1.s * delta2.t - delta2.s * delta1.t));

    // Calcular tangente e bitangente da face
    auto const tangent{glm::vec4(M[0][0] * e1.x + M[0][1] * e2.x,
                                 M[0][0] * e1.y + M[0][1] * e2.y,
                                 M[0][0] * e1.z + M[0][1] * e2.z, 0.0f)};

    auto const bitangent{glm::vec3(M[1][0] * e1.x + M[1][1] * e2.x,
                                   M[1][0] * e1.y + M[1][1] * e2.y,
                                   M[1][0] * e1.z + M[1][1] * e2.z)};

    // Acumular tangente nos vértices e bitangente no vetor auxiliar
    v1.tangent += tangent;
    v2.tangent += tangent;
    v3.tangent += tangent;

    bitangents.at(i1) += bitangent;
    bitangents.at(i2) += bitangent;
    bitangents.at(i3) += bitangent;
  }

  // Ortogonalizar as tangentes em relação às normais dos vértices
  for (auto &&[i, vertex] : iter::enumerate(m_vertices)) {
    auto const &n{vertex.normal};
    auto const &t{glm::vec3(vertex.tangent)};

    // Ortogonalizar a tangente em relação à normal
    auto const tangent{t - n * glm::dot(n, t)};
    vertex.tangent = glm::vec4(glm::normalize(tangent), 0);

    // Calcular handedness (orientação) da base ortogonalizada
    auto const b{glm::cross(n, t)};
    auto const handedness{glm::dot(b, bitangents.at(i))};
    vertex.tangent.w = (handedness < 0.0f) ? -1.0f : 1.0f;
  }
}

/**
 *  Cria e carrega buffers OpenGL para os vértices e índices do modelo.
 *
 * Esta função cria e carrega buffers OpenGL para armazenar os vértices e
 * índices do modelo. Ela realiza os seguintes passos:
 *   1. Deleta os buffers anteriores, se existirem.
 *   2. Gera e vincula um novo buffer de vértices (VBO) e carrega os dados dos
 * vértices.
 *   3. Gera e vincula um novo buffer de índices (EBO) e carrega os dados dos
 * índices.
 *
 *  Esta função assume que as informações dos vértices e índices do modelo já
 * foram carregadas e estão presentes nos membros de dados 'm_vertices' e
 * 'm_indices'. Além disso, espera-se que o contexto OpenGL esteja ativo antes
 * da chamada desta função.
 */
void Model::createBuffers() {
  // Deletar buffers anteriores, se existirem
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);

  // Gerar e carregar buffer de vértices (VBO)
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER,
                     sizeof(m_vertices.at(0)) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Gerar e carregar buffer de índices (EBO)
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices.at(0)) * m_indices.size(),
                     m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/**
 * Carrega uma textura de difusão (textura de cor) para o modelo.
 *
 * Esta função verifica se o arquivo de textura especificado por 'path' existe.
 * Se o arquivo existir, ela deleta a textura de difusão anterior e carrega a
 * nova textura utilizando a função de utilitário 'abcg::loadOpenGLTexture'.
 *
 *  path O caminho do arquivo da textura de difusão a ser carregada.
 *
 *  Se o arquivo de textura não existir, a função não realiza nenhuma ação.
 *       Antes de chamar esta função, é necessário garantir que o contexto
 * OpenGL esteja ativo.
 *
 */
void Model::loadDiffuseTexture(std::string_view path) {
  // Verificar se o arquivo de textura existe
  if (!std::filesystem::exists(path))
    return;

  // Deletar a textura de difusão anterior, se existir
  abcg::glDeleteTextures(1, &m_diffuseTexture);

  // Carregar a nova textura de difusão utilizando a função de utilitário
  m_diffuseTexture = abcg::loadOpenGLTexture({.path = path});
}

/**
 * Carrega um modelo OBJ a partir de um arquivo e inicializa os membros de dados
 * do objeto Model.
 *
 * Esta função carrega um modelo OBJ a partir de um arquivo especificado por
 * 'path' e realiza as seguintes etapas:
 *   1. Configura o caminho para buscar arquivos.
 *   2. Utiliza a biblioteca TinyObjLoader para parsear o arquivo OBJ e obter
 * atributos, formas e materiais.
 *   3. Limpa os vetores de vértices e índices do modelo.
 *   4. Itera sobre as formas e índices para construir os vértices e índices do
 * modelo.
 *   5. Configura as propriedades de material do modelo, como ambientes,
 * difusão, especularidade e textura.
 *   6. Se a opção 'standardize' for verdadeira, padroniza o modelo.
 *   7. Se o modelo não possui informações de normais, calcula automaticamente.
 *   8. Se o modelo possui coordenadas de textura, calcula as tangentes.
 *   9. Cria e carrega buffers OpenGL para os vértices e índices do modelo.
 *
 *  path O caminho do arquivo do modelo OBJ a ser carregado.
 *  standardize Uma flag indicando se o modelo deve ser padronizado após o
 * carregamento.
 *
 *  abcg::RuntimeError Se houver erros durante o carregamento do modelo.
 *
 *  Antes de chamar esta função, é necessário garantir que o contexto OpenGL
 * esteja ativo.
 *
 */
void Model::loadObj(std::string_view path, bool standardize) {
  // Configurar o caminho para buscar arquivos
  auto const basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  // Utiliza TinyObjLoader para parsear o arquivo OBJ
  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath; // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  // Tratar avisos durante o parsing
  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  // Obter atributos, formas e materiais do modelo
  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};
  auto const &materials{reader.GetMaterials()};

  // Limpar vetores de vértices e índices do modelo
  m_vertices.clear();
  m_indices.clear();

  // Reinicializa flags para normais e coordenadas de textura
  m_hasNormals = false;
  m_hasTexCoords = false;

  // Mapa chave-valor com chave=Vertex e valor=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Iterar sobre formas e índices para construir vértices e índices do modelo
  for (auto const &shape : shapes) {
    // Loop indices
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Vertex
      auto const index{shape.mesh.indices.at(offset)};

      // Posição
      auto const startIndex{3 * index.vertex_index};
      glm::vec3 position{attrib.vertices.at(startIndex + 0),
                         attrib.vertices.at(startIndex + 1),
                         attrib.vertices.at(startIndex + 2)};

      // Normal
      glm::vec3 normal{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        auto const normalStartIndex{3 * index.normal_index};
        normal = {attrib.normals.at(normalStartIndex + 0),
                  attrib.normals.at(normalStartIndex + 1),
                  attrib.normals.at(normalStartIndex + 2)};
      }

      // Texturas
      glm::vec2 texCoord{};
      if (index.texcoord_index >= 0) {
        m_hasTexCoords = true;
        auto const texCoordsStartIndex{2 * index.texcoord_index};
        texCoord = {attrib.texcoords.at(texCoordsStartIndex + 0),
                    attrib.texcoords.at(texCoordsStartIndex + 1)};
      }

      Vertex const vertex{
          .position = position, .normal = normal, .texCoord = texCoord};

      // Se não contém vertex
      if (!hash.contains(vertex)) {
        // adiciona index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // adiciona vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  // Configurar propriedades de material do modelo
  if (!materials.empty()) {
    auto const &mat{materials.at(0)}; // First material
    m_Ka = {mat.ambient[0], mat.ambient[1], mat.ambient[2], 1};
    m_Kd = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1};
    m_Ks = {mat.specular[0], mat.specular[1], mat.specular[2], 1};
    m_shininess = mat.shininess;

    if (!mat.diffuse_texname.empty())
      loadDiffuseTexture(basePath + mat.diffuse_texname);

  } else {
    // Configurar valores padrão se não houver material
    m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
    m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
    m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
    m_shininess = 25.0f;
  }

  // Padronizar o modelo, se necessário
  if (standardize) {
    Model::standardize();
  }

  // Calcular normais, se não existirem
  if (!m_hasNormals) {
    computeNormals();
  }

  // Calcular tangentes, se existirem coordenadas de textura
  if (m_hasTexCoords) {
    computeTangents();
  }

  // Criar e carregar buffers OpenGL para vértices e índices do modelo
  createBuffers();
}

/**
 *  Renderiza o modelo utilizando o VAO e a textura de difusão associada.
 *
 * Esta função assume que o contexto OpenGL já está ativo e realiza as seguintes
 * etapas:
 *   1. Vincula o VAO (Array de Objetos de Vértices) do modelo.
 *   2. Ativa a textura de difusão associada ao modelo no canal de textura
 * GL_TEXTURE0.
 *   3. Configura os parâmetros de filtragem para minificação e magnificação da
 * textura.
 *   4. Configura os parâmetros de repetição da textura nos eixos S e T.
 *   5. Renderiza o modelo usando a função abcg::glDrawElements.
 *   6. Desvincula o VAO para evitar interferências com outros objetos gráficos.
 *
 *  numTriangles Número opcional de triângulos a serem renderizados. Se for
 * menor que zero, renderiza todos os triângulos do modelo. Caso contrário,
 * renderiza o número especificado de triângulos multiplicado por 3 (para
 * triângulos OpenGL).
 *
 *  Antes de chamar esta função, é necessário garantir que o contexto OpenGL
 * esteja ativo.
 *
 *  Model::m_VAO, Model::m_diffuseTexture, abcg::glBindVertexArray,
 * abcg::glBindTexture, abcg::glTexParameteri, abcg::glDrawElements
 */
void Model::render(int numTriangles) const {
  // Vincular o VAO do modelo
  abcg::glBindVertexArray(m_VAO);

  // Ativar a textura de difusão associada ao modelo no canal GL_TEXTURE0
  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);

  // Configurar parâmetros de filtragem para minificação e magnificação da
  // textura
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Configurar parâmetros de repetição da textura nos eixos S e T
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Calcular o número total de índices a serem renderizados
  auto const numIndices{(numTriangles < 0) ? m_indices.size()
                                           : numTriangles * 3};

  // Renderizar o modelo usando a função abcg::glDrawElements
  abcg::glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  // Desvincular o VAO para evitar interferências com outros objetos gráficos
  abcg::glBindVertexArray(0);
}

/**
 *  Configura o VAO (Array de Objetos de Vértices) do modelo para uso com um
 * programa de shader OpenGL.
 *
 * Esta função realiza as seguintes etapas:
 *   1. Libera o VAO anterior, se existir.
 *   2. Gera um novo VAO e o vincula.
 *   3. Vincula o EBO (Buffer de Objetos de Elementos) e o VBO (Buffer de
 * Objetos de Vértices) do modelo.
 *   4. Vincula e configura os atributos de vértice para as variáveis do shader,
 * como posição, normal, coordenadas de textura e tangente.
 *   5. Desvincula o VAO e os buffers para evitar interferências com outros
 * objetos gráficos.
 *
 *  program O identificador do programa de shader OpenGL associado ao modelo.
 *
 *  Antes de chamar esta função, é necessário garantir que o contexto OpenGL
 * esteja ativo.
 *
 *  Model::m_VAO, Model::m_EBO, Model::m_VBO, abcg::glDeleteVertexArrays,
 * abcg::glGenVertexArrays, abcg::glBindVertexArray, abcg::glBindBuffer,
 * abcg::glEnableVertexAttribArray, abcg::glVertexAttribPointer
 */
void Model::setupVAO(GLuint program) {
  // Liberar o VAO anterior, se existir
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Gerar um novo VAO e vincula
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);

  // Vincular o EBO e o VBO do modelo
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Vincular e configurar os atributos de vértice para as variáveis do shader
  auto const positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), nullptr);
  }

  // Obter o identificador do atributo de vértice para a normal no programa de
  // shader

  auto const normalAttribute{abcg::glGetAttribLocation(program, "inNormal")};
  if (normalAttribute >= 0) {
    // Se o atributo de vértice para a normal existir no programa de shader
    // Habilitar o array de atributos de vértice
    abcg::glEnableVertexAttribArray(normalAttribute);

    // Definir o ponteiro do atributo de vértice para a normal
    auto const offset{offsetof(Vertex, normal)};
    abcg::glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  // Obter o identificador do atributo de vértice para as coordenadas de textura
  // no programa de shader
  auto const texCoordAttribute{
      abcg::glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    // Se o atributo de vértice para as coordenadas de textura existir no
    // programa de shader
    // Habilitar o array de atributos de vértice
    abcg::glEnableVertexAttribArray(texCoordAttribute);

    // Definir o ponteiro do atributo de vértice para as coordenadas de textura
    auto const offset{offsetof(Vertex, texCoord)};
    abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  // Obter o identificador do atributo de vértice para a tangente no programa de
  // shader
  auto const tangentCoordAttribute{
      abcg::glGetAttribLocation(program, "inTangent")};
  if (tangentCoordAttribute >= 0) {
    // Se o atributo de vértice para a tangente existir no programa de shader
    // Habilitar o array de atributos de vértice
    abcg::glEnableVertexAttribArray(tangentCoordAttribute);

    // Definir o ponteiro do atributo de vértice para a tangente
    auto const offset{offsetof(Vertex, tangent)};
    abcg::glVertexAttribPointer(tangentCoordAttribute, 4, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  // Desvincular o VAO e os buffers para evitar interferências com outros
  // objetos gráficos
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}

/**
 *  Padroniza as posições dos vértices do modelo para que estejam centradas na
 * origem e a maior dimensão esteja normalizada para o intervalo [-1, 1].
 *
 * Esta função realiza as seguintes etapas:
 *   1. Calcula os limites (máximo e mínimo) das coordenadas x, y e z dos
 * vértices do modelo.
 *   2. Calcula o centro dos limites.
 *   3. Calcula a escala necessária para normalizar a maior dimensão para [-1,
 * 1].
 *   4. Atualiza as posições dos vértices aplicando a transformação de
 * centralização e escala.
 *
 *  Esta função modifica diretamente as posições dos vértices no modelo.
 *
 *
 */
void Model::standardize() {
  // Calcular os limites das coordenadas x, y e z dos vértices do modelo
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (auto const &vertex : m_vertices) {
    max = glm::max(max, vertex.position);
    min = glm::min(min, vertex.position);
  }

  // Calcular o centro dos limites
  auto const center{(min + max) / 2.0f};

  // Calcular a escala necessária para normalizar a maior dimensão para [-1, 1]
  auto const scaling{2.0f / glm::length(max - min)};

  // Atualizar as posições dos vértices aplicando a transformação de
  // centralização e escala
  for (auto &vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}

/**
 *  Libera os recursos alocados para o modelo no contexto OpenGL.
 *
 * Esta função realiza as seguintes etapas:
 *   1. Deleta a textura de difusão associada ao modelo.
 *   2. Deleta o Buffer de Objetos de Elementos (EBO) do modelo.
 *   3. Deleta o Buffer de Objetos de Vértices (VBO) do modelo.
 *   4. Deleta o Array de Objetos de Vértices (VAO) do modelo.
 *
 *  Esta função deve ser chamada ao finalizar o uso do modelo para liberar os
 * recursos alocados.
 */
void Model::destroy() {
  // Deletar a textura de difusão associada ao modelo
  abcg::glDeleteTextures(1, &m_diffuseTexture);

  // Deletar o Buffer de Objetos de Elementos (EBO) do modelo
  abcg::glDeleteBuffers(1, &m_EBO);

  // Deletar o Buffer de Objetos de Vértices (VBO) do modelo
  abcg::glDeleteBuffers(1, &m_VBO);

  // Deletar o Array de Objetos de Vértices (VAO) do modelo
  abcg::glDeleteVertexArrays(1, &m_VAO);
}