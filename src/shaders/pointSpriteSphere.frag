#version 330

//!< in-variables
in vec4 passPosition;
in float passLifeBuffer;

//!< uniforms
uniform bool sphereColor;
uniform sampler2D tex;

//!< out-variables
out vec4 fragcolor;


bool inSphere(vec3 position, float sphereRadius)
{
return (sqrt((position.x*position.x) +(position.y*position.y)+(position.z*position.z))) <= sphereRadius;
}

void main()
{
	if (!sphereColor)
	{
		if (inSphere(passPosition.xyz, 0.3))
		{
		//fragcolor = vec4(1,0,0,1);
		fragcolor = texture(tex, gl_PointCoord);
		fragcolor.w *= passLifeBuffer;
		}
	else
	{
	//fragcolor = vec4(1,1,1,passLifeBuffer);
	fragcolor = texture(tex, gl_PointCoord);
	fragcolor.w *= passLifeBuffer;
	}
	}else
{
	fragcolor = vec4(0.1,0.1,0.1,1);
	//fragcolor = vec4(passPosition.x,passPosition.y,passPosition.z,1-passPosition.y);
}
}