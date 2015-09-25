#include <GL/glew.h>

//create a VBO
//target is usually GL_ARRAY_BUFFER 
//usage is usually GL_DYNAMIC_DRAW
GLuint createVBO(const void* data, int dataSize, GLenum target, GLenum usage);

