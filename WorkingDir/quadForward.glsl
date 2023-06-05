///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef FORWARD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
//layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBiTangent;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

struct Light
{
    int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D normals;
layout(location = 2) uniform sampler2D colors;
layout(location = 3) uniform sampler2D forwardColor;
layout(location = 4) uniform sampler2D depth;
layout(location = 5) uniform sampler2D bloom;

uniform int renderMode;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = vec4(texture(forwardColor, vTexCoord).rgb + texture(bloom, vTexCoord).rgb, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.