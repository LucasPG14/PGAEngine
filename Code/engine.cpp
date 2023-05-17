//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include "assimp_model_loading.h"
#include "buffermanagement.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace Utils
{
    u8 GetGLComponentCount(GLenum type)
    {
        switch (type)
        {
        case GL_FLOAT_VEC3:
            return 3;
        case GL_FLOAT_VEC2:
            return 2;
        default:
            assert("Not implemented");
        }
    }
}

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void ChargeProgram(Program& program)
{
    i32 attributeCount;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    for (int i = 0; i < attributeCount; ++i)
    {
        char attributeName[128] = {};

        GLsizei attributeNameLength = 0;
        GLsizei attributeSize = 0;
        GLenum attributeType = 0;
        glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName), &attributeNameLength, &attributeSize, &attributeType, attributeName);

        GLint location = glGetAttribLocation(program.handle, attributeName);

        VertexShaderAttribute attribute = {};
        attribute.location = location;
        attribute.componentCount = Utils::GetGLComponentCount(attributeType);
        program.vertexInputLayout.attributes.push_back(attribute);
    }
}

void Init(App* app)
{
    app->glInfo.glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    app->glInfo.glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    app->glInfo.glVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    app->glInfo.glShadingLanguage = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

    int numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (int i = 0; i < numExtensions; ++i)
    {
        app->glInfo.glExtensions.push_back(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
    }

    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3V2V), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3V2V), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);

    app->texturedGeometryProgramIdx = LoadProgram(app, "shaders.glsl", "MESH_GEOMETRY");
    Program& program = app->programs[app->texturedGeometryProgramIdx];
    ChargeProgram(program);

    app->deferredIdx = LoadProgram(app, "deferred.glsl", "DEFERRED");
    Program& program2 = app->programs[app->deferredIdx];
    ChargeProgram(program2);

    app->finalQuadIdx = LoadProgram(app, "finalQuad.glsl", "FINAL_QUAD");
    Program& program3 = app->programs[app->finalQuadIdx];
    ChargeProgram(program3);

    app->programUniformTexture = glGetUniformLocation(program.handle, "uTexture");

    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    // Uniform buffer
    app->uniformBuffer = CreateBuffer(app->maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STATIC_DRAW);
    app->globalParamsOffset = app->uniformBuffer.head;

    app->sphereIdx = LoadModel(app, "sphere/sphere.obj");

    for (int i = -1; i <= 1; ++i)
    {
        Entity& entity = app->entities.emplace_back();
        entity.modelIndex = LoadModel(app, "Patrick/Patrick.obj");
        entity.localParamsOffset = (sizeof(glm::mat4) * 2) * app->entities.size();
        entity.localParamsSize = sizeof(glm::mat4) * 2;

        entity.position = vec3(i * 5.0f, 0.0f, 0.0f);
        entity.rotation = vec3(0.0f);
        entity.scale = vec3(1.0f);
    }

    for (int i = 0; i < 3; ++i)
    {
        Light& light = app->lights.emplace_back();
        light.color = glm::vec3(1.0, 0.0, 0.0);
        light.position = glm::vec3(0.0, 0.0, 0.0);
        light.direction = glm::vec3(0.0, 0.0, 0.0);
        light.type = LightType::POINT;
    }

    glEnable(GL_DEPTH_TEST);

    app->fbo1 = new Framebuffer(3, app->displaySize.x, app->displaySize.y);

    app->mode = Mode_TexturedQuad;

    app->camera.Init({0.0f, 0.0f, 5.0f}, 0.1f, 1000.0f, app->displaySize.x / app->displaySize.y);
}

void Gui(App* app)
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("Render Mode"))
    {
        if (ImGui::MenuItem("FINAL RENDER", "", app->renderMode == RenderMode::FINAL_RENDER))
        {
            app->renderMode = RenderMode::FINAL_RENDER;
        }
        if (ImGui::MenuItem("POSITIONS", "", app->renderMode == RenderMode::POSITIONS))
        {
            app->renderMode = RenderMode::POSITIONS;
        }
        if (ImGui::MenuItem("NORMALS", "", app->renderMode == RenderMode::NORMALS))
        {
            app->renderMode = RenderMode::NORMALS;
        }
        if (ImGui::MenuItem("ALBEDO", "", app->renderMode == RenderMode::ALBEDO))
        {
            app->renderMode = RenderMode::ALBEDO;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    ImGui::Begin("Viewport");
    ImGui::Image((void*)app->fbo1->GetColorAttachment(2), {(float)app->displaySize.x, (float)app->displaySize.y}, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
    ImGui::End();

    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);

    if (ImGui::BeginPopup("OpenGL information"))
    {
        ImGui::Text("GL Version: ");
        ImGui::SameLine();
        ImGui::Text(app->glInfo.glVersion.c_str());
        ImGui::Text("GL Renderer: ");
        ImGui::SameLine();
        ImGui::Text(app->glInfo.glRenderer.c_str());
        ImGui::Text("GL Vendor: ");
        ImGui::SameLine();
        ImGui::Text(app->glInfo.glVendor.c_str());
        ImGui::Text("GL Shading Language: ");
        ImGui::SameLine();
        ImGui::Text(app->glInfo.glShadingLanguage.c_str());

        ImGui::Text("Num extensions: ");
        ImGui::SameLine();
        ImGui::Text(std::to_string(app->glInfo.glExtensions.size()).c_str());

        for (int i = 0; i < app->glInfo.glExtensions.size(); ++i)
        {
            ImGui::Text(app->glInfo.glExtensions[i].c_str());
        }

        ImGui::EndPopup();
    }

    for (int i = 0; i < app->entities.size(); ++i)
    {
        ImGui::PushID(i);

        Entity& entity = app->entities[i];

        ImGui::DragFloat3("Position", glm::value_ptr(entity.position));
        ImGui::DragFloat3("Scale", glm::value_ptr(entity.scale));

        entity.worldMatrix = glm::translate(entity.position);
        entity.worldMatrix = glm::scale(entity.worldMatrix, entity.scale);

        ImGui::PopID();
    }

    ImGui::End();

    ImGui::Begin("Lights");

    for (int i = 0; i < app->lights.size(); ++i)
    {
        ImGui::PushID(i);
        if (ImGui::CollapsingHeader(("Light " + std::to_string(i)).c_str()))
        {
            Light& light = app->lights[i];
            
            ImGui::DragFloat3("Position", glm::value_ptr(light.position));
            ImGui::DragFloat3("Direction", glm::value_ptr(light.direction));
            ImGui::ColorPicker4("Color", glm::value_ptr(light.color));
        }
        ImGui::PopID();
    }

    ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    app->camera.Update(app->input, app->deltaTime);

    MapBuffer(app->uniformBuffer, GL_WRITE_ONLY);

    app->globalParamsOffset = app->uniformBuffer.head;

    PushVec3(app->uniformBuffer, app->camera.GetPosition());
    PushUInt(app->uniformBuffer, app->lights.size());

    // Global Params
    for (int i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->uniformBuffer, sizeof(vec4));
        
        Light& light = app->lights[i];
        PushUInt(app->uniformBuffer, (u32)light.type);
        PushVec3(app->uniformBuffer, light.color);
        PushVec3(app->uniformBuffer, light.direction);
        PushVec3(app->uniformBuffer, light.position);
    }

    app->globalParamsSize = app->uniformBuffer.head - app->globalParamsOffset;
    
    // Local Params
    for (int i = 0; i < app->entities.size(); ++i)
    {
        AlignHead(app->uniformBuffer, app->uniformBlockAlignment);
        Entity& entity = app->entities[i];
        glm::mat4 worldViewProj = app->camera.GetViewProjection() * entity.worldMatrix;

        entity.localParamsOffset = app->uniformBuffer.head;
        PushMat4(app->uniformBuffer, entity.worldMatrix);
        PushMat4(app->uniformBuffer, worldViewProj);
        entity.localParamsSize = app->uniformBuffer.head - entity.localParamsOffset;
    }
    
    UnmapBuffer(app->uniformBuffer);
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    for (u32 i = 0; i < submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].handle == program.handle)
            return submesh.vaos[i].handle;
    }

    GLuint vaoHandle;
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset;
                    const u32 stride = submesh.vertexBufferLayout.stride;

                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked);
        }
    }

    glBindVertexArray(0);

    VertexArray vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);
    return vaoHandle;
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
            {
                // TODO: Draw your textured quad here!
                // - clear the framebuffer
                // - set the viewport
                // - set the blending state
                // - bind the texture into unit 0
                // - bind the program 
                //   (...and make its texture sample from unit 0)
                // - bind the vao
                // - glDrawElements() !!!

                app->fbo1->Bind();
                glClearColor(0.1, 0.1, 0.1, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glViewport(0, 0, app->displaySize.x, app->displaySize.y);
                Program& program = app->programs[app->deferredIdx];
                glUseProgram(program.handle);

                glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->uniformBuffer.handle, app->globalParamsOffset, app->globalParamsSize);
                
                for (int i = 0; i < app->entities.size(); ++i)
                {
                    Entity& entity = app->entities[i];

                    glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->uniformBuffer.handle, entity.localParamsOffset, entity.localParamsSize);

                    Model& model = app->models[entity.modelIndex];
                    Mesh& mesh = app->meshes[model.meshIdx];

                    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                    {
                        GLuint vao = FindVAO(mesh, i, program);
                        glBindVertexArray(vao);

                        u32 submeshMaterialIdx = model.materialIdx[i];
                        Material& submeshMaterial = app->materials[submeshMaterialIdx];

                        glUniform1i(app->programUniformTexture, 0);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);

                        Submesh& submesh = mesh.submeshes[i];
                        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    }
                    glBindVertexArray(0);
                }
                glUseProgram(0);
                app->fbo1->Unbind();

                glClearColor(0.1, 0.1, 0.1, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                //Program& programQuad = app->programs[app->finalQuadIdx];
                //glUseProgram(programQuad.handle);

                //glBindVertexArray(app->vao);
                //glEnable(GL_BLEND);
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                //glUniform1i(app->programUniformTexture, 0);
                //glActiveTexture(GL_TEXTURE0);
                //glBindTexture(GL_TEXTURE_2D, app->textures[app->diceTexIdx].handle);

                //app->fbo1->BindTextures();
                //GLint location = glGetUniformLocation(programQuad.handle, "positions");
                //glUniform1i(location, 0);
                //location = glGetUniformLocation(programQuad.handle, "normals");
                //glUniform1i(location, 1);
                //location = glGetUniformLocation(programQuad.handle, "colors");
                //glUniform1i(location, 2);
                
                //glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(u16), GL_UNSIGNED_SHORT, 0);
                //glUseProgram(0);
            }
            break;

        default:;
    }
}

