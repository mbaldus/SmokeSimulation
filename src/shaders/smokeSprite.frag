#version 330

//!< in-variables
in vec4 passPosition;
in float passLifeBuffer;
in float passDensityBuffer;
in float rndmSprite;

//!< uniforms
uniform bool sphereColor;
uniform sampler2D tex1;
uniform sampler2D tex2;

//!< out-variables
out vec4 fragcolor;

void main()
{
		fragcolor = texture(tex1, gl_PointCoord);
		fragcolor.w *= 2*passLifeBuffer;
		fragcolor.xyz -= 0.25*passDensityBuffer;
	
		if(passPosition.y < 0.35)
			{
			fragcolor = texture(tex1, gl_PointCoord);
			//fragcolor.w *= 2*passLifeBuffer;
			//fragcolor.xyz *= passDensityBuffer;
			}
	
	
	/*	if (rndmSprite == 0.0 )
		{
		fragcolor = texture(tex1, gl_PointCoord);
		fragcolor.w *= 2*passLifeBuffer;
		fragcolor.xyz -= 0.125*passDensityBuffer;
		} 
		else 
	 	if (rndmSprite == 1.0)
		{
		fragcolor = texture(tex2, gl_PointCoord);
		fragcolor.w *= 2*passLifeBuffer;
		fragcolor.xyz -= 0.125*passDensityBuffer;
		}else 
		fragcolor = vec4(0,0,0,1);
		
		//if(passPosition.y < 0.5)
			//{
			fragcolor = texture(tex1, gl_PointCoord);
			fragcolor.w *= 2*passLifeBuffer;
			//fragcolor.xyz *= passDensityBuffer;
			//}*/
}