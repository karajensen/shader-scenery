////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - opengltexture.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "opengltexture.h"
#include "opengl/soil/SOIL.h"

GlTexture::GlTexture(const std::string& filepath) :
    m_filepath(filepath)
{
}

GlTexture::~GlTexture()
{
    Release();
}

void GlTexture::Release()
{
}

void GlTexture::Initialise(GLuint id)
{
    m_id = id;
    glBindTexture(GL_TEXTURE_2D, m_id);

    int width, height;
    unsigned char* image = SOIL_load_image(m_filepath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void GlTexture::SendTexture()
{
    glBindTexture(GL_TEXTURE_2D, m_id);
}