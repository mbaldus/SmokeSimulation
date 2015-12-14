#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;
layout(loaction = 1) in float densityBuffer;
//layout(loaction = 2) in float lifeBuffer;

//!< out-variables

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 passPosition;
//out vec2 passUVCoord;

void main()
{
	gl_Position = projection * view * model * positionAttribute;
	passPosition = model * positionAttribute;
}