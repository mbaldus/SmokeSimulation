#version 330

 //!< in-variables
layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in float lifeBuffer;
layout(location = 2) in float densityBuffer;
layout(location = 3) in float rndmS;

//!< out-variables

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 passPosition;
out float passLifeBuffer;
out float passDensityBuffer;
out float rndmSprite;

void main()
{
	gl_Position = projection * view * model * positionAttribute;
	passPosition = model * positionAttribute;
	passLifeBuffer = lifeBuffer;
	passDensityBuffer = densityBuffer;
	rndmSprite = rndmS;

	gl_PointSize = 3*(0.5-lifeBuffer) * 150 / gl_Position.w; 
	//0.5 - 0.25 = 0.25 *150  = 3* 37.5 am Anfangs
	//0.5 - 0.0 = 0.5 * 150	  = 3* 75 am Ende
}