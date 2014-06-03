////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - openglengine.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "openglengine.h"
#include "openglshader.h"
#include "openglmesh.h"
#include "opengltexture.h"
#include "opengltarget.h"

/**
* Internal data for the opengl rendering engine
*/
struct OpenglData
{
    /**
    * Constructor
    */
    OpenglData();

    /**
    * Destructor
    */
    ~OpenglData();

    /**
    * Releases the device/context
    */
    void Release();

    HGLRC hrc;                       ///< Rendering context  
    HDC hdc;                         ///< Device context  
                                     
    GlRenderTarget backBuffer;       ///< Render target for the back buffer
    GlRenderTarget sceneTarget;      ///< Render target for the main scene
    GlRenderTarget normalTarget;     ///< Render target for the scene normal/depth map
    GlShader postShader;             ///< Shader for post processing the scene
    GlShader normalShader;           ///< Shader for rendering normals/depth for the scene
    GlMesh quad;                     ///< Quad to render the final post processed scene onto
                                     
    std::vector<GlTexture> textures; ///< Textures shared by all meshes
    std::vector<GlMesh> meshes;      ///< Each mesh in the scene
    std::vector<GlShader> shaders;   ///< Shaders shared by all meshes
    glm::vec3 camera;                ///< Position of the camera
    glm::mat4 projection;            ///< Projection matrix
    glm::mat4 view;                  ///< Camera View matrix
    bool isBackfaceCull;             ///< Whether backface culling is currently active
    int selectedShader;              ///< Currently active shader for rendering
    bool viewUpdated;                ///< Whether the view matrix has updated this tick
    bool lightsUpdated;              ///< Whether the lights have been updated this tick
};

OpenglData::OpenglData() :
    hdc(nullptr),
    hrc(nullptr),
    isBackfaceCull(true),
    selectedShader(NO_INDEX),
    viewUpdated(true),
    lightsUpdated(true),
    quad("ScreenQuad"),
    sceneTarget("SceneTarget"),
    normalTarget("NormalTarget"),
    backBuffer("BackBuffer", true),
    postShader(POST_VERT_FX, POST_FRAG_FX, POST_VERT_ASM, POST_FRAG_ASM),
    normalShader(NORM_VERT_FX, NORM_FRAG_FX, NORM_VERT_ASM, NORM_FRAG_ASM)
{
}

OpenglData::~OpenglData()
{
    Release();
}

void OpenglData::Release()
{
    viewUpdated = true;
    lightsUpdated = true;
    selectedShader = NO_INDEX;
    isBackfaceCull = true;

    for(GlTexture& texture : textures)
    {
        texture.Release();
    }

    for(GlMesh& mesh : meshes)
    {
        mesh.Release();
    }

    for(GlShader& shader : shaders)
    {
        shader.Release();
    }

    backBuffer.Release();
    sceneTarget.Release();
    normalTarget.Release();
    postShader.Release();
    normalShader.Release();
    quad.Release();

    wglMakeCurrent(nullptr, nullptr);
    if(hrc)
    {
        wglDeleteContext(hrc);
        hrc = nullptr;
    }
}

OpenglEngine::OpenglEngine(HWND hwnd, HINSTANCE hinstance) :
    m_data(new OpenglData()),
    m_hwnd(hwnd)
{
    // Create temporary window used only for Glew
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX)); 
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = nullptr;
    wc.hInstance = hinstance; 
    wc.lpszClassName = "GlewWindow";
    RegisterClassEx(&wc); 

    m_temporaryHwnd = CreateWindowEx(WS_EX_APPWINDOW,
        "GlewWindow", TEXT("GlewWindow"), 0, 0, 0, 0, 0, 
        nullptr, nullptr, hinstance, nullptr);
}

OpenglEngine::~OpenglEngine()
{
}

bool OpenglEngine::Initialize()
{
    // SetPixelFormat can only be called once per window and is required for Glew.
    // Initialise Glew with a temporary opengl context and temporary window.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd369049%28v=vs.85%29.aspx

    m_data->Release();
    HDC tempHdc = GetDC(m_temporaryHwnd);
    m_data->hdc = GetDC(m_hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int tempPixelFormat = ChoosePixelFormat(tempHdc, &pfd);
    if(tempPixelFormat == 0)
    {
        Logger::LogError("OpenGL: GLEW Pixel Format unsupported");
        return false;
    }
  
    if(!SetPixelFormat(tempHdc, tempPixelFormat, &pfd))
    {
        Logger::LogError("OpenGL: GLEW Set Pixel Format failed");
        return false;
    }

    HGLRC tempOpenGLContext = wglCreateContext(tempHdc); 
    wglMakeCurrent(tempHdc, tempOpenGLContext);
    if(!tempOpenGLContext || HasCallFailed())
    {
        Logger::LogError("OpenGL: GLEW Temporary context creation failed");
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK || HasCallFailed())
    {
        Logger::LogError("OpenGL: GLEW Initialization failed");
        return false;
    }

    if(!wglewIsSupported("WGL_ARB_create_context") || HasCallFailed())
    { 
        Logger::LogError("OpenGL: GLEW No support for 3.0+");
        return false;
    }  

    // Determine the supported attributes for opengl. Do this before deleting 
    // the temporary context as HasCallFailed() does not work without one
    const int pixelAttributes[] = 
    {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_SAMPLE_BUFFERS_ARB,     GL_TRUE,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_SUPPORT_OPENGL_ARB,     TRUE,
        WGL_STENCIL_BITS_ARB,       0,
        WGL_SAMPLES_ARB,            MULTISAMPLING_COUNT,
        0,                          0
    };

    UINT numFormats = 0;
    int pixelFormat = 0;
    wglChoosePixelFormatARB(m_data->hdc, pixelAttributes, 0, 1, &pixelFormat, &numFormats);
    if(pixelFormat == 0 || numFormats == 0 || HasCallFailed())
    {
        Logger::LogInfo("OpenGL: Choose pixel format failed");
        return false;
    }
    
    if(!SetPixelFormat(m_data->hdc, pixelFormat, &pfd))
    {
        Logger::LogInfo("OpenGL: Set pixel format failed");
        return false;
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempOpenGLContext);

    // Create the actual opengl context
    int contextAttributes[] = 
    {  
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_FLAGS_ARB,
        0, 0  
    };  

    m_data->hrc = wglCreateContextAttribsARB(m_data->hdc, 0, contextAttributes);
    wglMakeCurrent(m_data->hdc, m_data->hrc);
    if(!m_data->hrc || HasCallFailed())
    {
        Logger::LogError("OpenGL: Context creation failed");
        return false;
    }

    int minor, major;
    glGetIntegerv(GL_MAJOR_VERSION, &major); 
    glGetIntegerv(GL_MINOR_VERSION, &minor); 
    if(minor == -1 || major == -1)
    {
        Logger::LogError("OpenGL: Version not supported");
        return false;
    }

    // Create the render targets
    if(!m_data->backBuffer.Initialise() ||
       !m_data->sceneTarget.Initialise() ||
       !m_data->normalTarget.Initialise())
    {
        Logger::LogError("OpenGL: Could not create render targets");
        return false;
    }

    // Create the normal/depth shader
    std::string errorBuffer = m_data->normalShader.CompileShader();
    if(!errorBuffer.empty())
    {
        Logger::LogError("OpenGL: Normal shader failed: " + errorBuffer);
        return false;
    }

    // Create the post processing quad and shader
    if(!m_data->quad.Initialise())
    {
        Logger::LogError("OpenGL: Scene quad failed to initialise");
        return false;
    }

    errorBuffer = m_data->postShader.CompileShader();
    if(!errorBuffer.empty())
    {
        Logger::LogError("OpenGL: Post shader failed: " + errorBuffer);
        return false;
    }

    if(!m_data->postShader.HasTextureSlot(SCENE_TEXTURE) ||
       !m_data->postShader.HasTextureSlot(NORMAL_TEXTURE))
    {
        Logger::LogError("OpenGL: Post shader does not have required texture slots");
        return false;
    }

    // Initialise the opengl environment
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glFrontFace(GL_CCW); 
    glMatrixMode(GL_PROJECTION);

    m_data->projection = glm::perspective(FIELD_OF_VIEW, 
        WINDOW_WIDTH / static_cast<float>(WINDOW_HEIGHT),
        CAMERA_NEAR, CAMERA_FAR);

    if(!HasCallFailed())
    {
        std::stringstream stream;
        stream << "OpenGL: Verson " << major << "." << minor << " successful";
        Logger::LogInfo(stream.str());
        return true;
    }

    return false;
}

std::string OpenglEngine::CompileShader(int index)
{
    return m_data->shaders[index].CompileShader();
}

bool OpenglEngine::InitialiseScene(const std::vector<Mesh>& meshes, 
                                   const std::vector<Mesh>& alpha, 
                                   const std::vector<Shader>& shaders,
                                   const std::vector<Texture>& textures)
{
    m_data->textures.reserve(textures.size());
    for(const Texture& texture : textures)
    {
        m_data->textures.push_back(GlTexture(texture.path));
    }

    m_data->shaders.reserve(shaders.size());
    for(const Shader& shader : shaders)
    {
        m_data->shaders.push_back(GlShader(shader.index, 
            shader.glslVertexFile, shader.glslFragmentFile));
    }

    m_data->meshes.reserve(meshes.size());
    for(const Mesh& mesh : meshes)
    {
        m_data->meshes.push_back(GlMesh(&mesh));
    }

    return ReInitialiseScene();
}

bool OpenglEngine::ReInitialiseScene()
{
    for(unsigned int i = 0; i < m_data->shaders.size(); ++i)
    {
        const std::string result = CompileShader(i);
        if(!result.empty())
        {
            Logger::LogError("OpenGL: " + result, true);
            return false;
        }
    }

    for(GlMesh& mesh : m_data->meshes)
    {
        if(!mesh.Initialise())
        {
            Logger::LogError("OpenGL: Failed to re-initialise mesh");
            return false;
        }
    }

    for(GlTexture& texture : m_data->textures)
    {
        if(!texture.Initialise())
        {
            Logger::LogError("OpenGL: Failed to re-initialise texture");
            return false;
        }
    }

    return true;
}

void OpenglEngine::BeginRender()
{
    m_data->selectedShader = NO_INDEX; // always reset due to post shader
    m_data->lightsUpdated = true; // updated once per tick due to tweaking
}

void OpenglEngine::Render(const std::vector<Light>& lights)
{
    // Render the scene
    m_data->sceneTarget.SetActive();
    for(GlMesh& mesh : m_data->meshes)
    {
        UpdateShader(mesh.GetShaderID(), lights);
        SetTextures(mesh.GetTextureIDs());
        SetBackfaceCull(mesh.ShouldBackfaceCull());
    
        mesh.PreRender();
        m_data->shaders[m_data->selectedShader].EnableAttributes();
        mesh.Render();
    }

    // Render the scene normal/depth map
    m_data->normalTarget.SetActive();
    m_data->normalShader.SetActive();
    for(GlMesh& mesh : m_data->meshes)
    {
        SetBackfaceCull(mesh.ShouldBackfaceCull());
        mesh.PreRender();
        m_data->normalShader.EnableAttributes();
        mesh.Render();
    }

    // Render the scene as a texture to the backbuffer
    m_data->backBuffer.SetActive();
    m_data->postShader.SetActive();
    m_data->sceneTarget.SendTexture(SCENE_TEXTURE);
    m_data->normalTarget.SendTexture(NORMAL_TEXTURE);
    m_data->quad.PreRender();
    m_data->postShader.EnableAttributes();
    m_data->quad.Render();
    m_data->sceneTarget.ClearTexture(SCENE_TEXTURE);
    m_data->normalTarget.ClearTexture(NORMAL_TEXTURE);
}

void OpenglEngine::SetTextures(const std::vector<int>& textureIDs)
{
    GlShader& shader = m_data->shaders[m_data->selectedShader];

    int slot = 0;
    for(int id : textureIDs)
    {
        if(id != NO_INDEX)
        {
            if(shader.HasTextureSlot(slot))
            {
                m_data->textures[id].SendTexture(slot++);
            }
            else
            {
                Logger::LogError("Shader and mesh texture count does not match");
            }
        }
    }
}

void OpenglEngine::UpdateShader(int index, const std::vector<Light>& lights)
{
    bool changedShader = false;
    if(index != m_data->selectedShader)
    {
        m_data->shaders[index].SetActive();
        m_data->selectedShader = index;
        changedShader = true;
    }
    GlShader& shader = m_data->shaders[m_data->selectedShader];

    // Update transform information
    if(changedShader || m_data->viewUpdated)
    {
        // Model pivot points exist at the origin: world matrix is the identity
        shader.SendUniformMatrix("viewProjection",  m_data->projection * m_data->view);
        shader.SendUniformFloat("cameraPosition", &m_data->camera.x, 3);
        m_data->viewUpdated = false;
    }
    
    // Update light information
    if(changedShader || m_data->lightsUpdated)
    {
        shader.SendUniformFloat("lightPosition", &lights[0].position.x, 3);
        m_data->lightsUpdated = false;
    }
}

void OpenglEngine::EndRender()
{
    SwapBuffers(m_data->hdc); 
}

std::string OpenglEngine::GetName() const
{
    return "OpenGL";
}

void OpenglEngine::UpdateView(const Matrix& world)
{
    glm::mat4 view;

    view[0][0] = world.m11;  
    view[1][0] = world.m12;
    view[2][0] = world.m13;
    view[3][0] = world.m14;

    view[0][1] = world.m21;
    view[1][1] = world.m22;
    view[2][1] = world.m23;
    view[3][1] = world.m24;

    view[0][2] = world.m31;
    view[1][2] = world.m32;
    view[2][2] = world.m33;
    view[3][2] = world.m34;

    m_data->camera.x = world.m14;
    m_data->camera.y = world.m24;
    m_data->camera.z = world.m34;

    m_data->viewUpdated = true;
    m_data->view = glm::inverse(view);
}

void OpenglEngine::SetBackfaceCull(bool shouldCull)
{
    if(shouldCull != m_data->isBackfaceCull)
    {
        m_data->isBackfaceCull = shouldCull;
        shouldCull ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    }
}