#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in float lifeBuffer;
layout(location = 2) in float densityBuffer;

//!< out-variables

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 passPosition;
out float passLifeBuffer;
out float passDensityBuffer;

void main()
{
	gl_Position = projection * view * model * positionAttribute;
	passPosition = model * positionAttribute;
	passLifeBuffer = lifeBuffer;
	passDensityBuffer = densityBuffer;

	gl_PointSize = 75 / gl_Position.w;
}