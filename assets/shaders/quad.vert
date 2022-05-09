#version 330

layout (location = 0) in vec2 posCoords;
layout (location = 1) in vec4 vertRgbaColor;
layout (location = 2) in vec2 vertTexCoords;
layout (location = 3) in float vertTexIndex;
layout (location = 4) in float vertMapIndex;
layout (location = 5) in vec2 vertMapMod;

out vec4 rgbaColor;
out vec2 texCoords;
out float texIndex;
out float mapIndex;
out vec2 mapMod;

uniform mat4 MVP;

void main()
{
    rgbaColor = vertRgbaColor;
    texCoords = vertTexCoords;
    texIndex = vertTexIndex;
    mapIndex = vertMapIndex;
    mapMod = vertMapMod;
    // mLod = vertLod;
    
    gl_Position = MVP * vec4(posCoords, 0.0, 1.0);
}
