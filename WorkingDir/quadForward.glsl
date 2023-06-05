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
layout(location = 4) uniform sampler2D forwardColor;
layout(location = 5) uniform sampler2D depth;
layout(location = 6) uniform sampler2D bloom;

uniform int renderMode;
uniform int hdrActive;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(location = 0) out vec4 oColor;

void main()
{
    if (renderMode == 0)
    {
        vec3 colorFinal = texture(forwardColor, vTexCoord).rgb;
        if (hdrActive == 0)
        {
            const float gamma = 2.2;
  
            // reinhard tone mapping
            vec3 mapped = colorFinal / (colorFinal + vec3(1.0));
            // gamma correction 
            mapped = pow(mapped, vec3(1.0 / gamma));
  
            oColor = vec4(mapped, 1.0);
            oColor += vec4(texture(bloom, vTexCoord).rgb, 1.0);
        }
        else
        {
            oColor += vec4(colorFinal, 1.0);
            oColor += vec4(texture(bloom, vTexCoord).rgb, 1.0);
        }
    }
    if (renderMode == 1)
    {
        oColor = vec4(texture(positions, vTexCoord).rgb, 1.0);
    }
    if (renderMode == 2)
    {
        oColor = vec4(texture(normals, vTexCoord).rgb, 1.0);
    }
    if (renderMode == 3)
    {
        oColor = vec4(texture(colors, vTexCoord).rgb, 1.0);
    }
    if (renderMode == 4)
    {
        oColor = vec4(vec3(texture(depth, vTexCoord).r), 1.0);
    }
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.