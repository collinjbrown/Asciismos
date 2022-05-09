// This just contains the data we'll use in animation_2D.cpp.
// Nothing too fancy, again.

#ifndef ANIMATION_2D_H
#define ANIMATION_2D_H

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "texture_2D.h"

class Animation2D
{
private:
    Animation2D();

public:
    GLuint              ID;
    unsigned int        width;
    unsigned int        height;
    unsigned int        columns;
    unsigned int        rows;
    float               speed;
    bool                loop;
    std::vector<int>    rowsToCols;
    GLuint              internalFormat;
    GLuint              imageFormat;
    GLuint              wrapS;
    GLuint              wrapT;
    GLuint              filterMin;
    GLuint              filterMax;

    Animation2D(const char* file, bool alpha, int columns, int rows, float speed, std::vector<int> rowsToColls, bool loop, int filter = GL_LINEAR);

    void bind() const;
};

#endif