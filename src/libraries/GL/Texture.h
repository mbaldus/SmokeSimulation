#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Texture
{
public:
   Texture(std::string path);
   ~Texture();

   GLuint getHandle();

   GLuint load(std::string path);

protected:
	GLuint m_textureHandle;
};


#endif //TEXTURE_H
