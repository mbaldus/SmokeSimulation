#version 330

//!< in-variables
in vec4 passPosition;
in float passLifeBuffer;
in float passDensityBuffer;
in float rndmSprite;

//!< uniforms
uniform sampler2D smokeTextures[10];

//!< out-variables
out vec4 fragcolor;

void main()
{
		int texNr = int(rndmSprite+0.1);
		fragcolor = texture(smokeTextures[texNr], gl_PointCoord);
		fragcolor.w *= 0.2*passLifeBuffer; //0.2 * [0,1] -> [0,0.2]
		fragcolor.xyz -= 0.375*passDensityBuffer;
}