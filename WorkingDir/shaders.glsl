///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef MESH_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBiTangent;

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;
out vec3 vViewDir;

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
    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    vNormal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;
    vViewDir = uCameraPosition - vPosition;

    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
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
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(location=0) out vec4 oColor;

vec3 CalcDirectionalLight(vec3 direction, vec3 color)
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

vec3 CalcPointLight(vec3 position, vec3 color)
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

    //float distance = length(position - vPosition);
    //float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * (distance * distance));

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 color = texture(uTexture, vTexCoord).rgb;

    for (int i = 0; i < uLightCount; ++i)
    {
        vec3 result;
        if (uLights[i].type == 0)
        {
            result = CalcDirectionalLight(uLights[i].direction, uLights[i].color) * color;
        }
        else if (uLights[i].type == 1)
        {
            result = CalcPointLight(uLights[i].position, uLights[i].color) * color;
        }

        oColor += vec4(result, 1.0);
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