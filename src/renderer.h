#ifndef RENDERER_H
#define RENDERER_H

// Renderer.h just contains all the data we're gonna need for renderer.cpp to do its job.
// There might be a bug in here right now, having to do with max quad count, but we'll
// fix that at some point in the future (if it is, in fact, a problem, which it may well not be).

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
// #include "texture_2D.h"
#include "animation_2D.h"

class GlobalPositionComponent;
class ColliderComponent;

struct Vertex
{
    float xCoord;
    float yCoord;

    float rColor;
    float gColor;
    float bColor;
    float aColor;

    float sCoord;
    float tCoord;

    float textureIndex;
    float mapIndex;

    float widthMod;
    float heightMod;
};

struct Quad
{
    Vertex topRight;
    Vertex bottomRight;
    Vertex bottomLeft;
    Vertex topLeft;
};

class Bundle
{
public:
    int batch;
    float textureLocation;
    float mapLocation;
};

// Store the quads before a draw call
class Batch
{
public:
    static constexpr int MAX_QUADS = 10000;

    // TODO: Look into decoupling # of quads that can be rendered with # of textures that can be rendered in one batch
    std::array<Quad, MAX_QUADS> quadBuffer;
    int quadIndex = 0;
};

// A batch renderer for quads with a color and sprite
class Renderer
{
public:
    // Should be GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS for release, but
    // would need to figure out how to use that value in the fragment shader.
    // NOTE: Fragment shader also has hard-coded value that must match this.
    static constexpr int MAX_TEXTURES_PER_BATCH = 32;

    std::vector<GLuint> textureIDs;
    std::vector<GLuint> texturesUsed;
    float whiteTextureIndex;

    GLuint VAO;
    GLuint VBO;

    GLuint whiteTextureID;

    Renderer(GLuint whiteTexture);
    float CalculateModifier(float i);
    void CloseOffBatch();
    Bundle DetermineBatch(int textureID, int mapID);
    void prepareQuad(GlobalPositionComponent* pos, float width, float height, float scaleX, float scaleY, glm::vec4 rgb, int textureID, int mapID, bool tiled, bool flippedX, bool flippedY);
    void prepareQuad(GlobalPositionComponent* pos, ColliderComponent* col, float width, float height, float scaleX, float scaleY, glm::vec4 rgb, int textureID, int mapID);
    void prepareQuad(glm::vec2 topRight, glm::vec2 bottomRight, glm::vec2 bottomLeft, glm::vec2 topLeft, glm::vec4 rgb, float scaleX, float scaleY, int textureID, int mapID);
    void prepareQuad(glm::vec2 position, float width, float height, float scaleX, float scaleY, glm::vec4 rgb, int textureID, int mapID); // Specify texture ID rather than index?
    // NOTE: Directly sending a texture index rather than ID can result in the wrong texture being drawn (due to being in the wrong batch)
    void prepareQuad(GlobalPositionComponent* pos, float width, float height, float scaleX, float scaleY, glm::vec4 rgb, int animID, int mapID, int cellX, int cellY, int cols, int rows, bool flippedX, bool flippedY);
    void prepareQuad(int batchIndex, Quad& input);
    void prepareDownLine(float x, float y, float height);
    void prepareRightLine(float x, float y, float width);
    void sendToGL();
    void resetBuffers();

private:
    std::vector<Batch> batches;
    Shader shader;

    void flush(const Batch& batch);
};

#endif
