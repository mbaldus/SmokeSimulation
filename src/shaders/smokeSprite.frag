#version 330

//!< in-variables
in vec4 passPosition;
in float passLifeBuffer;
in float passDensityBuffer;

//!< uniforms
uniform bool sphereColor;
uniform sampler2D tex;

//!< out-variables
out vec4 fragcolor;

void main()
{
		fragcolor = texture(tex, gl_PointCoord);
		//fragcolor.w *= 2*passLifeBuffer;
		fragcolor.w *= 2*passLifeBuffer;
		//fragcolor = vec4(1.0-fragcolor.x,1.0-fragcolor.y,1.0-fragcolor.z,passLifeBuffer);
		fragcolor.xyz *= passDensityBuffer;
}