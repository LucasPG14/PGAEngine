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

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLights[16];
};

layout(location = 0) out vec4 positions;
layout(location = 1) out vec4 normals;
layout(location = 2) out vec4 colors;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float bumpiness = -1.0;

    float minLayers = 10.0;
    float maxLayers = 32.0;
    const float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 1.0 / numLayers;

    float currentLayerDepth = 0.0;

    vec2 P = viewDir.xy / viewDir.z * bumpiness;
    vec2 deltaTexCoords = P / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthTexture, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        
        currentDepthMapValue = texture(depthTexture, currentTexCoords).r;  
   
        currentLayerDepth += layerDepth;  
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthTexture, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    
    return finalTexCoords;
}

void main()
{
    vec3 viewDir = normalize(vTangentViewPos - vTangentFragPos);
    vec2 texCoords = ParallaxMapping(vTexCoord,  viewDir);

    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
    
    positions = vec4(vPosition, 1.0);
    vec3 normal = texture(normalTexture, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normals = vec4(normal, 1.0);

    colors = vec4(texture(uTexture, texCoords).rgb, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.