#include "renderer.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "check_error.h"
#include "game.h"
#include "component.h"

// This holds all the functions we use to send rendering info to OpenGL.
// In short, one calls some variation on prepareQuad() from outside (like in ecs.cpp)
// and then this handles the rest. It takes that quad and determines which batch
// the textures are in, ensuring that the texture and its map aren't accidentally placed
// in separate batches. Then, it calculates the modifier that is sent to the fragment shader
// which is necessary to get texture sampling to work correctly. At the end of each frame,
// Main calls sendToGL() which in turn flushes out however many batches we need
// and then Main calls resetBuffers() to prepare for the next frame.

float Renderer::CalculateModifier(float i)
{
    return (256.0f * (1.0f / i));
}

void Renderer::CloseOffBatch()
{
    int diff = texturesUsed.size() % MAX_TEXTURES_PER_BATCH + 1;

    for (int i = 0; i < diff; i++)
    {
        texturesUsed.push_back(0);
    }
}

Bundle Renderer::DetermineBatch(int textureID, int mapID)
{
    auto result = std::find(texturesUsed.rbegin(), texturesUsed.rend(), textureID);
    int location;
    if (result != texturesUsed.rend())
    {
        // The texture has been used.
        location = texturesUsed.rend() - result;
    }
    else
    {
        // The texture has not yet been used.
        location = -1;
    }

    auto resultMap = std::find(texturesUsed.rbegin(), texturesUsed.rend(), mapID);
    int locationMap;
    if (resultMap != texturesUsed.rend())
    {
        // The map has been used.
        locationMap = texturesUsed.rend() - resultMap;
    }
    else
    {
        // The map has not yet been used.
        locationMap = -1;
    }

    int textureBatch = floor(location / MAX_TEXTURES_PER_BATCH);
    int mapBatch = floor(locationMap / MAX_TEXTURES_PER_BATCH);

    Bundle bundle;
    int currentBatch = floor(texturesUsed.size() / MAX_TEXTURES_PER_BATCH);
    // std::cout << std::to_string(currentBatch) + "\n";

    if (textureBatch == mapBatch && location != -1 && locationMap != -1)
    {
        // They're both already used and in the same batch.
        return bundle = { textureBatch, (float)(static_cast<int>(location - 1) % MAX_TEXTURES_PER_BATCH), (float)(static_cast<int>(locationMap - 1) % MAX_TEXTURES_PER_BATCH) };
    }
    else if (textureBatch != mapBatch && location != -1 && locationMap != -1)
    {
        // They're both already used and in different batches.

        if (texturesUsed.size() % MAX_TEXTURES_PER_BATCH > MAX_TEXTURES_PER_BATCH - 2)
        {
            // They're both already used and in different batches and the current batch lacks space for them.
            CloseOffBatch();
            int newBatch = floor(texturesUsed.size() / MAX_TEXTURES_PER_BATCH);
            texturesUsed.push_back(textureID);
            texturesUsed.push_back(mapID);
            return bundle = { newBatch, 0, 1 };
        }
        else
        {
            // They're both already used and in different batches and the current batch has space for them.
            texturesUsed.push_back(textureID);
            texturesUsed.push_back(mapID);
            return bundle = { currentBatch, static_cast<float>(texturesUsed.size()) - 2 % MAX_TEXTURES_PER_BATCH, static_cast<float>(texturesUsed.size()) - 1 % MAX_TEXTURES_PER_BATCH };

        }
    }
    else if (textureBatch != mapBatch && locationMap == -1 && textureBatch == currentBatch)
    {
        // Only the texture has already been used.
        if (texturesUsed.size() % MAX_TEXTURES_PER_BATCH != 0)
        {
            // Only the texture has already been used in the current batch and it has space for the map.
            texturesUsed.push_back(mapID);
            return bundle = { currentBatch, static_cast<float>(location) - 1, static_cast<float>(texturesUsed.size()) - 1 };
        }
        else
        {
            // Only the texture has already been used in the current batch and it lacks space for the map.
            CloseOffBatch();
            int newBatch = floor(texturesUsed.size() / MAX_TEXTURES_PER_BATCH);
            texturesUsed.push_back(textureID);
            texturesUsed.push_back(mapID);
            return bundle = { newBatch, 0, 1 };
        }
    }
    else if (textureBatch != mapBatch && location == -1 && mapBatch == currentBatch)
    {
        // Only the map has already been used.
        if (texturesUsed.size() % MAX_TEXTURES_PER_BATCH != 0)
        {
            // Only the map has already been used in the current batch and it has space for the texture.
            texturesUsed.push_back(textureID);
            return bundle = { currentBatch, static_cast<float>(texturesUsed.size()) - 1, static_cast<float>(locationMap) - 1 };
        }
        else
        {
            // Only the map has already been used in the current batch and it lacks space for the texture.
            CloseOffBatch();
            int newBatch = floor(texturesUsed.size() / MAX_TEXTURES_PER_BATCH);
            texturesUsed.push_back(textureID);
            texturesUsed.push_back(mapID);
            return bundle = { newBatch, 0, 1 };
        }
    }
    else if (textureBatch == mapBatch && location == -1)
    {
        // Neither have been used and the current batch has space for both.
        texturesUsed.push_back(textureID);
        texturesUsed.push_back(mapID);
        return bundle = { currentBatch, (float)((static_cast<int>(texturesUsed.size()) - 2) % MAX_TEXTURES_PER_BATCH), (float)((static_cast<int>(texturesUsed.size() - 1)) % MAX_TEXTURES_PER_BATCH) };
    }
    else
    {
        // Neither have been used and the current batch lacks space for both.
        // Or some other mixture which needs fixing.
        CloseOffBatch();
        int newBatch = floor(texturesUsed.size() / MAX_TEXTURES_PER_BATCH);
        texturesUsed.push_back(textureID);
        texturesUsed.push_back(mapID);
        return bundle = { newBatch, 0, 1 };
    }
}

Renderer::Renderer(GLuint whiteTexture) : batches(1), shader("assets/shaders/quad.vert", "assets/shaders/quad.frag"), whiteTextureID(whiteTexture)
{
    GLuint quadIBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glGenBuffers(1, &quadIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);

    glCheckError();

    glBufferData(GL_ARRAY_BUFFER, Batch::MAX_QUADS * sizeof(Quad), nullptr, GL_DYNAMIC_DRAW);

    glCheckError();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, xCoord));
    glEnableVertexAttribArray(0);
    // rgba values for color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, rColor));
    glEnableVertexAttribArray(1);
    // s and t coordinates for texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, sCoord));
    glEnableVertexAttribArray(2);
    // Texture Index
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureIndex));
    glEnableVertexAttribArray(3);
    // Map Index
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, mapIndex));
    glEnableVertexAttribArray(4);
    // Dimensions Mod = 256 * (1 / [height or width])
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, widthMod));
    glEnableVertexAttribArray(5);
    glCheckError();

    // unsigned int quadVertices[] = {
    //     0, 1, 3,
    //     1, 2, 3,
    //     4, 5, 7,
    //     5, 6, 7
    // };
    unsigned int quadIndices[Batch::MAX_QUADS * 6];
    for (int i = 0; i < Batch::MAX_QUADS; i++)
    {
        const int rightOffset = 4 * i;
        const int leftOffset = 6 * i;

        quadIndices[leftOffset + 0] = rightOffset + 0;
        quadIndices[leftOffset + 1] = rightOffset + 1;
        quadIndices[leftOffset + 2] = rightOffset + 3;

        quadIndices[leftOffset + 3] = rightOffset + 1;
        quadIndices[leftOffset + 4] = rightOffset + 2;
        quadIndices[leftOffset + 5] = rightOffset + 3;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(shader.ID);
    GLint location = glGetUniformLocation(shader.ID, "batchQuadTextures");
    int samplers[MAX_TEXTURES_PER_BATCH];
    for (int i = 0; i < MAX_TEXTURES_PER_BATCH; i++)
    {
        samplers[i] = i;
    }
    glUniform1iv(location, MAX_TEXTURES_PER_BATCH, samplers);

    // Use white texture as the first texture
    // -----------------------------------------
    this->textureIDs.push_back(whiteTexture);
    this->texturesUsed.push_back(whiteTextureID);
    whiteTextureIndex = 0.0f;
}

void Renderer::prepareQuad(glm::vec2 position, float width, float height, float scaleX, float scaleY,
    glm::vec4 rgb, int textureID, int mapID)
{
    // Figure out which batch should be written to
    // -------------------------------------------
    Bundle bundle = DetermineBatch(textureID, mapID);
    Batch& batch = batches[bundle.batch];

    // Initialize the data for the quad
    // --------------------------------
    Quad& quad = batch.quadBuffer[batch.quadIndex];
    batch.quadIndex++;

    const float rightX = position.x + ((width * scaleX) / 2.0f);
    const float leftX = position.x - ((width * scaleX) / 2.0f);
    const float topY = position.y + ((height * scaleY) / 2.0f);
    const float bottomY = position.y - ((height * scaleY) / 2.0f);

    const float r = rgb.r;
    const float g = rgb.g;
    const float b = rgb.b;
    const float a = rgb.a;

    quad.topRight = { rightX, topY,      r, g, b, a,   1.0, 1.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.bottomRight = { rightX, bottomY,   r, g, b, a,   1.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.bottomLeft = { leftX,  bottomY,   r, g, b, a,   0.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.topLeft = { leftX,  topY,      r, g, b, a,   0.0, 1.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
}

void Renderer::prepareQuad(PositionComponent* pos, float width, float height, float scaleX, float scaleY,
    glm::vec4 rgb, int textureID, int mapID, bool tiled, bool flippedX, bool flippedY)
{
    // Figure out which batch should be written to
    // -------------------------------------------
    Bundle bundle = DetermineBatch(textureID, mapID);
    Batch& batch = batches[bundle.batch];

    float xL = 0.0f;
    float yL = 0.0f;
    float xR = 1.0f;
    float yR = 1.0f;

    if (flippedX)
    {
        xL = 1.0f;
        xR = 0.0f;
    }

    if (flippedY)
    {
        yL = 1.0f;
        yR = 0.0f;
    }

    // Initialize the data for the quad
    // --------------------------------

    const glm::vec2 topRight = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(((width * scaleX) / 2.0f), ((height * scaleY) / 2.0f)));
    const glm::vec2 bottomRight = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(((width * scaleX) / 2.0f), -((height * scaleY) / 2.0f)));
    const glm::vec2 bottomLeft = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(-((width * scaleX) / 2.0f), -((height * scaleY) / 2.0f)));
    const glm::vec2 topLeft = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(-((width * scaleX) / 2.0f), ((height * scaleY) / 2.0f)));

    const float r = rgb.r;
    const float g = rgb.g;
    const float b = rgb.b;
    const float a = rgb.a;

    if (tiled)
    {
        Quad& quad = batch.quadBuffer[batch.quadIndex];
        batch.quadIndex++;

        const float xMod = fmod(width, width); // tWidth);
        const float yMod = fmod(height, height); // tHeight);

        quad.topRight = { topRight.x, topRight.y,      r, g, b, a,   xMod, yMod,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
        quad.bottomRight = { bottomRight.x, bottomRight.y,   r, g, b, a,   xMod, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
        quad.bottomLeft = { bottomLeft.x,  bottomLeft.y,   r, g, b, a,   0.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
        quad.topLeft = { topLeft.x,  topLeft.y,      r, g, b, a,   0.0, yMod,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    }
    else
    {
        Quad& quad = batch.quadBuffer[batch.quadIndex];
        batch.quadIndex++;

        quad.topRight = { topRight.x, topRight.y,      r, g, b, a,   xR, yR,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
        quad.bottomRight = { bottomRight.x, bottomRight.y,   r, g, b, a,   xR, yL,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
        quad.bottomLeft = { bottomLeft.x,  bottomLeft.y,   r, g, b, a,   xL, yL,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
        quad.topLeft = { topLeft.x,  topLeft.y,      r, g, b, a,   xL, yR,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    }
}


void Renderer::prepareQuad(PositionComponent* pos, float width, float height, float scaleX, float scaleY,
    glm::vec4 rgb, int animID, int mapID, int cellX, int cellY, int cols, int rows, bool flippedX, bool flippedY)
{
    // Figure out which batch should be written to
    // -------------------------------------------
    Bundle bundle = DetermineBatch(animID, mapID);
    Batch& batch = batches[bundle.batch];

    
    // Figure out how cells should be handled.
    // ---------------------------------------
    float cellXMod = 1.0f / cols;
    float cellYMod = 1.0f / rows;

    float uvX0 =  cellX * cellXMod;
    float uvY0 = cellY * cellYMod;
    float uvX1 =  uvX0 + cellXMod;
    float uvY1 = uvY0 + cellYMod;

    if (flippedX)
    {
        float tempX0 = uvX0;

        uvX0 = uvX1;
        uvX1 = tempX0;
    }

    if (flippedY)
    {
        float tempY0 = uvY0;

        uvY0 = uvY1;
        uvY1 = tempY0;
    }

    /*std::cout << std::to_string(uvX0) + "/" + std::to_string(uvY0) + "\n";
    std::cout << std::to_string(uvX1) + "/" + std::to_string(uvY1) + "\n";*/

    // Initialize the data for the quad
    // --------------------------------
    Quad& quad = batch.quadBuffer[batch.quadIndex];
    batch.quadIndex++;

    const glm::vec2 topRight = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(((width * scaleX) / (float)cols), ((height * scaleY) / (float)rows)));
    const glm::vec2 bottomRight = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(((width * scaleX) / (float)cols), -((height * scaleY) / (float)rows)));
    const glm::vec2 bottomLeft = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(-((width * scaleX) / (float)cols), -((height * scaleY) / (float)rows)));
    const glm::vec2 topLeft = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(-((width * scaleX) / (float)cols), ((height * scaleY) / (float)rows)));

    const float r = rgb.r;
    const float g = rgb.g;
    const float b = rgb.b;
    const float a = rgb.a;

    float w = width / cols;
    float h = height / rows;

    quad.topRight = { topRight.x, topRight.y,      r, g, b, a,   uvX1, uvY1,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(w), CalculateModifier(h) };
    quad.bottomRight = { bottomRight.x, bottomRight.y,   r, g, b, a,   uvX1, uvY0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(w), CalculateModifier(h) };
    quad.bottomLeft = { bottomLeft.x,  bottomLeft.y,   r, g, b, a,   uvX0, uvY0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(w), CalculateModifier(h) };
    quad.topLeft = { topLeft.x,  topLeft.y,      r, g, b, a,   uvX0, uvY1,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(w), CalculateModifier(h) };
}


void Renderer::prepareQuad(PositionComponent* pos, ColliderComponent* col, float width, float height, float scaleX, float scaleY,
    glm::vec4 rgb, int textureID, int mapID)
{
    // Figure out which batch should be written to
    // -------------------------------------------
    Bundle bundle = DetermineBatch(textureID, mapID);
    Batch& batch = batches[bundle.batch];

    // Initialize the data for the quad
    // --------------------------------
    Quad& quad = batch.quadBuffer[batch.quadIndex];
    batch.quadIndex++;

    const glm::vec2 topRight = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(((width * scaleX) / 2.0f), ((height * scaleY) / 2.0f)));
    const glm::vec2 bottomRight = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(((width * scaleX) / 2.0f), -((height * scaleY) / 2.0f)));
    const glm::vec2 bottomLeft = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(-((width * scaleX) / 2.0f), -((height * scaleY) / 2.0f)));
    const glm::vec2 topLeft = glm::vec2(pos->x, pos->y) + pos->Rotate(glm::vec2(-((width * scaleX) / 2.0f), ((height * scaleY) / 2.0f)));

    const float r = rgb.r;
    const float g = rgb.g;
    const float b = rgb.b;
    const float a = rgb.a;

    quad.topRight = { topRight.x, topRight.y,      r, g, b, a,   1.0, 1.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.bottomRight = { bottomRight.x, bottomRight.y,   r, g, b, a,   1.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.bottomLeft = { bottomLeft.x,  bottomLeft.y,   r, g, b, a,   0.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.topLeft = { topLeft.x,  topLeft.y,      r, g, b, a,   0.0, 1.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
}

void Renderer::prepareQuad(glm::vec2 topRight, glm::vec2 bottomRight, glm::vec2 bottomLeft, glm::vec2 topLeft,
    glm::vec4 rgb, float scaleX, float scaleY, int textureID, int mapID)
{
    // Figure out which batch should be written to
    // -------------------------------------------
    Bundle bundle = DetermineBatch(textureID, mapID);
    Batch& batch = batches[bundle.batch];

    // Initialize the data for the quad
    // --------------------------------
    Quad& quad = batch.quadBuffer[batch.quadIndex];
    batch.quadIndex++;

    const float r = rgb.r;
    const float g = rgb.g;
    const float b = rgb.b;
    const float a = rgb.a;

    float width = topRight.x - topLeft.x;
    float height = topRight.y - bottomRight.y;

    quad.topRight = { topRight.x * scaleX, topRight.y * scaleY,      r, g, b, a,   1.0, 1.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.bottomRight = { bottomRight.x * scaleX, bottomRight.y * scaleY,   r, g, b, a,   1.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.bottomLeft = { bottomLeft.x * scaleX,  bottomLeft.y * scaleY,   r, g, b, a,   0.0, 0.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
    quad.topLeft = { topLeft.x * scaleX,  topLeft.y * scaleY,      r, g, b, a,   0.0, 1.0,    bundle.textureLocation, bundle.mapLocation, CalculateModifier(width), CalculateModifier(height) };
}

void Renderer::prepareQuad(int batchIndex, Quad& input)
{
    Batch& batch = batches[batchIndex];
    batch.quadBuffer[batch.quadIndex] = input;
    batch.quadIndex++;
}

void Renderer::sendToGL()
{
    shader.use();
    shader.setMatrix("MVP", Game::main.projection * Game::main.view);

    int currentBatch = 0;
    int texUnit = 0;

    for (int i = 0; i < texturesUsed.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(GL_TEXTURE_2D, textureIDs[texturesUsed[i] - 1]);

        if (texUnit >= MAX_TEXTURES_PER_BATCH - 1)
        {
            flush(batches[currentBatch]);

            currentBatch++;
            texUnit = 0;
        }
        else
        {
            texUnit++;
        }
    }

    flush(batches[currentBatch]);
}

void Renderer::prepareDownLine(float x, float y, float height)
{
    constexpr float halfWidth = 0.5f;
    Quad quad;
    quad.topRight = { x + halfWidth, y,            1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    quad.bottomRight = { x + halfWidth, y - height,   1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    quad.bottomLeft = { x - halfWidth, y - height,   1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    quad.topLeft = { x - halfWidth, y,            1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    prepareQuad(0, quad);
}

void Renderer::prepareRightLine(float x, float y, float width)
{
    constexpr float halfHeight = 0.5f;
    Quad quad;
    quad.topRight = { x + width, y + halfHeight, 1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    quad.bottomRight = { x + width, y - halfHeight, 1.0f, 1.0f, 1.0f, 1.0f,   1.0f, 0.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    quad.bottomLeft = { x        , y - halfHeight, 1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 0.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    quad.topLeft = { x        , y + halfHeight, 1.0f, 1.0f, 1.0f, 1.0f,   0.0f, 1.0f,    whiteTextureIndex, whiteTextureIndex, 8, 8 };
    prepareQuad(0, quad);
}

void Renderer::flush(const Batch& batch)
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Must bind VBO before glBufferSubData
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch.quadIndex * sizeof(Quad), &batch.quadBuffer[0]);
    glDrawElements(GL_TRIANGLES, batch.quadIndex * 6, GL_UNSIGNED_INT, nullptr);
}

void Renderer::resetBuffers()
{
    texturesUsed.clear();
    texturesUsed.push_back(whiteTextureID);

    for (Batch& batch : batches)
    {
        batch.quadIndex = 0;
    }
}
