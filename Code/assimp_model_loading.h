#pragma once

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "engine.h"

#include "platform.h"
#include "RenderStructs.h"

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);
void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
u32 LoadModel(App* app, const char* filename);
