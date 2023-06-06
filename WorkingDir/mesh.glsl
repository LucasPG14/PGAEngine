///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBiTangent;

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vTangentFragPos;
out vec3 vTangentViewPos;
out vec3 vNormal;
out vec3 vViewDir;
out mat3 tbn;

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
    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    vTexCoord = aTexCoord;
    vNormal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;
    vViewDir = uCameraPosition - vPosition;

    vec3 t = normalize(mat3(uWorldMatrix) * aTangent);
    vec3 b = normalize(mat3(uWorldMatrix) * aBiTangent);
    vec3 n = normalize(mat3(uWorldMatrix) * aNormal);
    tbn = transpose(mat3(t, b, n));

    vTangentViewPos = tbn * uCameraPosition;
    vTangentFragPos = tbn * vPosition;

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
in vec3 vTangentFragPos;
in vec3 vTangentViewPos;
in mat3 tbn;

layout(location = 0) uniform sampler2D uTexture;
layout(location = 1) uniform sampler2D normalTexture;
layout(location = 2) uniform sampler2D depthTexture;

uniform int renderMode;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(location = 0) out vec4 positions;
layout(location = 1) out vec4 normals;
layout(location = 2) out vec4 colors;
layout(location = 3) out vec4 brightColor;
layout(location = 4) out vec4 forwardColor;

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

vec3 CalcPointLight(vec3 position, vec3 color, vec3 fragPosition, vec3 normal)
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * color;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(position - fragPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(uCameraPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * color;  

    float distance = length(position - fragPosition);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{ 
    positions = vec4(vPosition, 1.0);
    vec3 normal = normalize(vNormal * 2.0 - 1.0);
    normals = vec4(normal, 1.0);

    colors = vec4(texture(uTexture, vTexCoord).rgb, 1.0);
    
    float brightness = dot(colors.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.0)
        brightColor = vec4(colors.rgb, 1.0);
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);

    if (renderMode == 0)
    {
        vec3 result;
        for (int i = 0; i < uLightCount; ++i)
        {
            if (uLights[i].type == 0)
            {
                result += CalcDirectionalLight(uLights[i].direction, uLights[i].color, vPosition, normal) * colors.rgb;
            }
            else if (uLights[i].type == 1)
            {
                result += CalcPointLight(uLights[i].position, uLights[i].color, vPosition, normal) * colors.rgb;
            }

        }
        forwardColor = vec4(result, 1.0);
    }
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.