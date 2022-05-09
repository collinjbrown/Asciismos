// This contains some of the data we'll need in texture_2D.cpp.
// Yeah, that's about it.

#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture2D
{
private:
    Texture2D();

public:
    static Texture2D* whiteTexture();

    GLuint       ID;
    unsigned int width;
    unsigned int height;
    GLuint       internalFormat;
    GLuint       imageFormat;
    GLuint       wrapS;
    GLuint       wrapT;
    GLuint       filterMin;
    GLuint       filterMax;

    Texture2D(const char* file, bool alpha, int filter = GL_LINEAR);

    void bind() const;
};

#endif
