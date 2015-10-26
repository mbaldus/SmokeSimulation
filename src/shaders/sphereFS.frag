#version 330

//!< in-variables
in vec4 passPosition;

//!< uniforms

//!< out-variables
out vec4 fragcolor;


bool inSphere(vec3 position, float sphereRadius)
{
return (sqrt((position.x*position.x) +(position.y*position.y)+(position.z*position.z))) <= sphereRadius;
}

void main()
{
	if (inSphere(passPosition.xyz, 0.3))
	{
		fragcolor = vec4(1,0,0,1);
	}
	else
	{
	fragcolor = vec4(1,1,1,1-passPosition.y+1);
	}
	//fragcolor = vec4(passPosition.x,passPosition.y,passPosition.z,1-passPosition.y);
}