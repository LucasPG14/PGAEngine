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

vec3 CalcDirectionalLight(Light dirLight, vec3 normal, vec3 viewDirection)
{
	vec3 lightDir = normalize(dirLight.direction);
			
	// Diffuse light
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * dirLight.color;
			
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * dirLight.color;

	// Specular light
    vec3 specularStrength = vec3(0.5);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 128.0);
	vec3 specular = spec * dirLight.color * specularStrength;

	return diffuse + ambient + specular;
}

vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 viewDirection, vec3 fragPos)
{
	vec3 ambient = vec3(0.1);

	vec3 lightDir = normalize(pointLight.position - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDirection);
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * pointLight.color;

	// Specular light
    vec3 specularStrength = vec3(0.5);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 128.0);
	vec3 specular = spec * pointLight.color * specularStrength;
	
	float distance = length(pointLight.position - fragPos);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;     

	return diffuse + ambient + specular;
}
void main()
{
    if (renderMode == 0)
    {
        vec3 color = texture(colors, vTexCoord).rgb;
        vec3 positionFrag = texture(positions, vTexCoord).rgb;
        vec3 normalFrag = normalize(texture(normals, vTexCoord).rgb);

        vec3 viewDir = normalize(uCameraPosition - positionFrag);

        vec3 result;
        for (int i = 0; i < uLightCount; ++i)
        {
            if (uLights[i].type == 0)
            {
                result += CalcDirectionalLight(uLights[i], normalFrag, viewDir) * color;
            }
            else if (uLights[i].type == 1)
            {
                result += CalcPointLight(uLights[i], normalFrag, viewDir, positionFrag) * color;
            }
        }

        if (hdrActive == 0)
        {
            const float gamma = 2.2;
  
            // reinhard tone mapping
            vec3 mapped = result / (result + vec3(1.0));
            // gamma correction 
            mapped = pow(mapped, vec3(1.0 / gamma));
  
            oColor = vec4(mapped, 1.0);
            oColor += vec4(texture(bloom, vTexCoord).rgb, 1.0);
        }
        else
        {
            oColor += vec4(result, 1.0);
            oColor += vec4(texture(bloom, vTexCoord).rgb, 1.0);
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
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.