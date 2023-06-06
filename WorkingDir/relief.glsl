///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef RELIEF

#if defined(VERTEX) ///////////////////////////////////////////////////

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

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBiTangent;

out vec3 fragPos;
out vec2 texCoords;
out vec3 tangentViewPos;
out vec3 tangentFragPos;
out mat3 tbn;

out vec3 t;
out vec3 b;
out vec3 n;

uniform vec3 viewPos;

void main()
{
    fragPos = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	texCoords = aTexCoord;

	vec3 T = normalize(mat3(uWorldMatrix) * aTangent);
	vec3 B = normalize(mat3(uWorldMatrix) * aBiTangent);
	vec3 N = normalize(mat3(uWorldMatrix) * aNormal);
	tbn = mat3(T, B, N);

    t = T;
    b = B;
    n = N;
	
    tangentViewPos = tbn * viewPos;
	tangentFragPos = tbn * fragPos;
	
	//vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0));
	
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 fragPos;
in vec2 texCoords;
in vec3 tangentViewPos;
in vec3 tangentFragPos;
in mat3 tbn;

in vec3 t;
in vec3 b;
in vec3 n;

layout(location = 0) uniform sampler2D uTexture;
layout(location = 1) uniform sampler2D normalTexture;
layout(location = 2) uniform sampler2D depthTexture;

uniform int renderMode;
uniform float minLayers;
uniform float maxLayers;
uniform float heightScale;

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

layout(location = 0) out vec4 positions;
layout(location = 1) out vec4 normals;
layout(location = 2) out vec4 colors;
layout(location = 3) out vec4 brightColor;
layout(location = 4) out vec4 forwardColor;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    const float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0;

	vec2 P = viewDir.xy / viewDir.z * heightScale;
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

vec3 CalcDirectionalLight(Light dirLight, vec3 normal, vec3 viewDirection)
{
	vec3 lightDir = normalize(dirLight.direction);
	
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * dirLight.color;
	
	float ambientStrength = 0.1;
	vec3 ambientLight = ambientStrength * dirLight.color;
	
	float specularStrength = 0.5;
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 128.0);
	vec3 specularLight = specularStrength * spec * dirLight.color;

	return diffuse + ambientLight + specularLight;
}

vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 viewDirection)
{
	vec3 lightDir = normalize((pointLight.position * tbn) - tangentFragPos);
	vec3 halfwayDir = normalize(lightDir + viewDirection);

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * pointLight.color;
	
	float ambientStrength = 0.1;
	vec3 ambientLight = ambientStrength * pointLight.color;
	
	float specularStrength = 0.5;
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 128.0);
	vec3 specularLight = specularStrength * spec * pointLight.color;

	float distance = length(pointLight.position - fragPos);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

	ambientLight *= attenuation; 
	diffuse *= attenuation;
	specularLight *= attenuation;   

	return diffuse + ambientLight + specularLight;
}

void main()
{ 
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec2 newTexCoords = ParallaxMapping(texCoords, viewDir);
    
    if(newTexCoords.x > 1.0 || newTexCoords.y > 1.0 || newTexCoords.x < 0.0 || newTexCoords.y < 0.0)
	{
		discard;
	}

    vec3 normal = texture(normalTexture, newTexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 color = texture(uTexture, newTexCoords).rgb;

    if (renderMode == 0)
    {
        vec3 result;
        for (int i = 0; i < uLightCount; ++i)
        {
            if (uLights[i].type == 0)
            {
                result += CalcDirectionalLight(uLights[i], normal, viewDir) * color;
            }
            else if (uLights[i].type == 1)
            {
                result += CalcPointLight(uLights[i], normal, viewDir) * color;
            }
        }
        forwardColor = vec4(result, 1.0);
    }

    colors = vec4(color.rgb, 1.0);
    
    float brightness = dot(colors.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.0)
        brightColor = vec4(colors.rgb, 1.0);
    else
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);

    positions = vec4(fragPos, 1.0);
    normals = vec4(b, 1.0);

    //specularColor.rgb = texture(uTexture, newTexCoords).rgb;
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.