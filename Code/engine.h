//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "RenderStructs.h"
#include "Camera.h"
#include "Framebuffer.h"
#include <glad/glad.h>

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    VertexShaderLayout vertexInputLayout;
    u64                lastWriteTimestamp; // What is this for?
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

enum class RenderMode
{
    FORWARD = 0,
    DEFERRED = 1
};

enum class TextureToRender
{
    FINAL_RENDER = 0,
    POSITIONS = 1,
    NORMALS = 2,
    ALBEDO = 3,
    DEPTH = 4
};

struct OpenGLInfo
{
    std::string glVersion;
    std::string glRenderer;
    std::string glVendor;
    std::string glShadingLanguage;
    std::vector<std::string> glExtensions;
};

struct Entity
{
    vec3 position;
    vec3 rotation;
    vec3 scale;
    glm::mat4 worldMatrix;
    
    u32 modelIndex;
    u32 localParamsOffset;
    u32 localParamsSize;

    bool relief;
};

struct Buffer
{
    GLuint handle;
    GLenum type;
    u32 size;
    u32 head;
    void* data;
};

enum class LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};

struct Light
{
    LightType type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

struct Vertex3V2V
{
    glm::vec3 pos;
    glm::vec2 uv;
};

const Vertex3V2V vertices[] = 
{
    {glm::vec3(-1.0, -1.0, 0.0), glm::vec2(0.0, 0.0)},
    {glm::vec3(1.0, -1.0, 0.0), glm::vec2(1.0, 0.0)},
    {glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 1.0)},
    {glm::vec3(-1.0, 1.0, 0.0), glm::vec2(0.0, 1.0)},
};

const u16 indices[] =
{
    0,1,2,
    0,2,3
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;
    std::vector<Program> programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 deferredIdx;
    u32 finalQuadIdx;
    u32 lightsIdx;
    u32 bloomIdx;
    u32 quadForwardIdx;
    u32 reliefIdx;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 diffuseWallTexIdx;
    u32 normalMapTexIdx;
    u32 depthMapTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;
    GLuint normalsUniformTexture;
    GLuint depthUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    OpenGLInfo glInfo;

    Camera camera;

    std::vector<Entity> entities;
    std::vector<Light> lights;

    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer uniformBuffer;
    Buffer cBuffer;
    u32 globalParamsOffset;
    u32 globalParamsSize;

    Framebuffer* fbo1;
    
    Framebuffer* fboBloom1;
    Framebuffer* fboBloom2;

    TextureToRender textureToRender;
    RenderMode renderMode;

    bool hdr = true;

    float minLayers = 10.0f;
    float maxLayers = 32.0f;
    float heightScale = -0.3f;

    u32 sphereIdx;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

u32 LoadTexture2D(App* app, const char* filepath);