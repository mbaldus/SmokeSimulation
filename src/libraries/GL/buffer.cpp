#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

#include <GL/buffer.h>

GLuint createVBO(const void* data, int dataSize,GLenum target)
{
	GLuint id = 0; //0 is reserved

	glGenBuffers(1, &id);	//create VBO
	glBindBuffer(target,id); //activate vbo id to use
	glBufferData(target, dataSize, data, GL_STATIC_DRAW); //upload data to video card

	//check data size in VBO is same as input array, if not return 0 and delete VBO
	int buffersize = 0;
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &buffersize);
	if(dataSize != buffersize)
	{
		glDeleteBuffers(1, &id);
		id = 0;
		printf("[createVBO()] Data size is mismatch with input array\n");
	}
	glBindBuffer(target,0);
	return id; //return VBO id
}




