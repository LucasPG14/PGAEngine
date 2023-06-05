///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef BLOOM

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

layout(location = 0) out vec4 oColor;

uniform bool horizontal;
uniform sampler2D image;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

uniform float lokkieras;

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, vTexCoord).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, vTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, vTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, vTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, vTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    oColor = vec4(result, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.