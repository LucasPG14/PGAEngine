///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef DEFERRED

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
//layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBiTangent;

out vec2 vTexCoord;

struct Light
{
    int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light
{
    int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

in vec2 vTexCoord;

layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D normals;
layout(location = 2) uniform sampler2D colors;
layout(location = 3) uniform sampler2D depth;

uniform int renderMode;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(location = 0) out vec4 oColor;

vec3 CalcDirectionalLight(vec3 direction, vec3 color, vec3 vPosition, vec3 vNormal)
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * color;

    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(-direction);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(uCameraPosition - vPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * color;  

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(vec3 position, vec3 color, vec3 vPosition, vec3 vNormal)
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * color;

    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(position - vPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(uCameraPosition - vPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * color;  

    float distance = length(position - vPosition);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
    if (renderMode == 0)
    {
        vec3 color = texture(colors, vTexCoord).rgb;
        vec3 positionFrag = texture(positions, vTexCoord).rgb;
        vec3 normalFrag = normalize(texture(normals, vTexCoord).rgb);

        for (int i = 0; i < uLightCount; ++i)
        {
            vec3 result;
            if (uLights[i].type == 0)
            {
                result = CalcDirectionalLight(uLights[i].direction, uLights[i].color, positionFrag, normalFrag) * color;
            }
            else if (uLights[i].type == 1)
            {
                result = CalcPointLight(uLights[i].position, uLights[i].color, positionFrag, normalFrag) * color;
            }

            oColor += vec4(result, 1.0);
        }
    }
    else if (renderMode == 1)
    {
        oColor = vec4(texture(positions, vTexCoord).rgb, 1.0);
    }
    else if (renderMode == 2)
    {
        oColor = vec4(texture(normals, vTexCoord).rgb, 1.0);
    }
    else if (renderMode == 3)
    {
        oColor = vec4(texture(colors, vTexCoord).rgb, 1.0);
    }
    else if (renderMode == 4)
    {
        oColor = vec4(vec3(texture(depth, vTexCoord).r), 1.0);
    }

    //oColor = vec4(result, 1.0);
    //oColor = vec4(uLights[0].color, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.