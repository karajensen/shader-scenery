////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - renderdata.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "mesh.h"
#include "water.h"
#include "shader.h"
#include "texture.h"
#include "postprocessing.h"
#include "emitter.h"
#include "light.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int MULTISAMPLING_COUNT = 4;
const int MAX_ANISOTROPY = 16;
const float FRUSTRUM_NEAR = 1.0f;
const float FRUSTRUM_FAR = 2000.0f;
const float FIELD_OF_VIEW = 60.0f;

const std::string SHADER_EXTENSION(".fx");
const std::string ASM_EXTENSION(".as");
const std::string GLSL_VERTEX("_glsl_vert");
const std::string GLSL_FRAGMENT("_glsl_frag");
const std::string HLSL_SHADER("_hlsl");
const std::string GLSL_FRAGMENT_EXTENSION(GLSL_FRAGMENT + SHADER_EXTENSION);   
const std::string GLSL_VERTEX_EXTENSION(GLSL_VERTEX + SHADER_EXTENSION);
const std::string HLSL_SHADER_EXTENSION(HLSL_SHADER + SHADER_EXTENSION);

const std::string ASSETS_PATH(".//Assets//");
const std::string MESH_PATH(ASSETS_PATH + "Meshes//");
const std::string SHADER_PATH(ASSETS_PATH + "Shaders//");
const std::string TEXTURE_PATH(ASSETS_PATH + "Textures//");
const std::string GENERATED_PATH(SHADER_PATH + "Generated//");
const std::string NORMAL_SHADER("normal");
const std::string BASE_SHADER("shader");

/**
* Index for special shaders
*/
enum ShaderIndex
{
    POST_SHADER_INDEX,
    BLUR_SHADER_INDEX,
    WATER_SHADER_INDEX,
    WATER_NORMAL_SHADER_INDEX,
    PARTICLE_SHADER_INDEX
};