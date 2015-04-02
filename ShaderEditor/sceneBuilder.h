////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - sceneBuilder.h
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "boost/property_tree/ptree.hpp"

class AnimatedTexture;
class FragmentLinker;
class Diagnostic;
class Scene;
class Mesh;
class Light;
class Texture;
class Shader;
class Water;
class Terrain;
class MeshData;

/**
* Builds all objects and diagnostics for the scene
*/
class SceneBuilder
{
public:

    SceneBuilder(Scene& scene);
    ~SceneBuilder();

    /**
    * Initialises the scene
    * @return Whether the initialization was successful
    */
    bool Initialise();

    /**
    * Outputs the scene to an xml file
    */
    void SaveSceneToFile();

    /**
    * Outputs the procedural texture to file
    * @param ID The ID of the texture to save
    */
    void SaveTextureToFile(int ID);

private:

    /**
    * Outputs post processing to an xml file
    */
    void SavePostProcessingtoFile();

    /**
    * Outputs the meshes and emitters to an xml file
    */
    void SaveMeshesToFile();

    /**
    * Outputs the lights to an xml file
    */
    void SaveLightsToFile();

    /**
    * Outputs particle emitters to an xml file
    */
    void SaveParticlesToFile();

    /**
    * Initiliases any stand-alone and shared shaders explicitly
    * @param linker The fragment linker used to generate shaders
    * @return Whether the initialization was successful
    */
    bool InitialiseShaders(FragmentLinker& linker);

    /**
    * Initialises the post processing for the final image
    * @return Whether the initialization was successful
    */
    bool InitialisePost();

    /**
    * Initialises the lighting for the scene
    * @return Whether the initialization was successful
    */
    bool InitialiseLighting();

    /**
    * Initialises the emitters for the scene
    * @return Whether the initialization was successful
    */
    bool InitialiseEmitters();

    /**
    * Initialises the diagnostics in the scene
    * @return Whether the initialization was successful
    */
    bool InitialiseDiagnostics();

    /**
    * Initialises all textures required
    * @return Whether the initialization was successful
    */
    bool InitialiseTextures();

    /**
    * Initialises the meshes for the scene
    * @param linker The fragment linker used to generate shaders
    * @return Whether the initialization was successful
    */
    bool InitialiseMeshes(FragmentLinker& linker);

    /**
    * Initialises a mesh shader for the scene
    * @param mesh The mesh to initialise
    * @param linker The fragment linker used to generate shaders
    */
    void InitialiseMeshShader(MeshData& mesh, FragmentLinker& linker);

    /**
    * Initialises any textures requires for the mesh
    * @param mesh The mesh to initialise
    */
    void InitialiseMeshTextures(MeshData& mesh);

    /**
    * Initialises a mesh for the scene
    * @param mesh The mesh to initialise
    * @param linker The fragment linker used to generate shaders
    * @return if initialization was successfull
    */
    bool InitialiseMesh(Mesh& mesh, FragmentLinker& linker);

    /**
    * Initialises terrain for the scene
    * @param terrain The terrain to initialise
    * @param linker The fragment linker used to generate shaders
    * @return if initialization was successfull
    */
    bool InitialiseTerrain(Terrain& terrain, FragmentLinker& linker);

    /**
    * Initialises a water mesh for the scene
    * @param water The water to initialise
    * @return if initialization was successfull
    */
    bool InitialiseWater(Water& water);

    /**
    * Adds a texture from a mesh if it doesn't already exist
    * @param name The name of the texture to add
    * @return The unique id of the texture added
    */
    int AddTexture(const std::string& name);

    /**
    * Gets the index for the shader if possible
    * @param shadername The name of the shader
    * @return the index for the shader or -1 if can't find
    */
    int GetShaderIndex(const std::string& shadername);

    /**
    * Gets the index for the shader
    * @param linker The fragment linker
    * @param shaderName The name of the shader to get the index for
    * @param meshName The name ofthe mesh the shader will be used for
    * @return the index for the shader
    */
    int GetShaderIndex(FragmentLinker& linker, 
                       const std::string& shadername, 
                       const std::string& meshName);

    Scene& m_scene; ///< The scene to build
};                     