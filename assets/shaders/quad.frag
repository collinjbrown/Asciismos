#version 330 core

in vec4 rgbaColor;
in vec2 texCoords;
in float texIndex;
in float mapIndex;
in vec2 mapMod;

out vec4 color;

// The size of this array is hard-coded,
// and must be manually changed if the QuadRenderer's
// corresponding constant is changed.

uniform sampler2D batchQuadTextures[32];

void main()
{
    int tIndex = int(texIndex);
    int mIndex = int(mapIndex);
    
    // ivec2 mapSize = textureSize(batchQuadTextures[mIndex], lod);

    vec4 sourceColor = texture(batchQuadTextures[tIndex], texCoords);
    vec2 mapCoord = vec2(sourceColor.r * mapMod.x,  sourceColor.g * mapMod.y);

    color = rgbaColor * texture(batchQuadTextures[mIndex], mapCoord);
    // color = vec4(sourceColor.r * mapSize.x, sourceColor.g * mapSize.y, 0.0, 1.0);
}
