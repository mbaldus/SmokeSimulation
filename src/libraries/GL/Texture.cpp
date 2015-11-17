#include "Texture.h"

Texture::Texture(std::string path)
{
	m_textureHandle = load(path);
}

Texture::~Texture()
{
    glDeleteTextures(1,&m_textureHandle);
}

GLuint Texture::getHandle()
{
	return m_textureHandle;
}

GLuint Texture::load(std::string path)
{

}