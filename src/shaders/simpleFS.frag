#version 330

//!< in-variables
in vec4 passPosition;

//!< uniforms

//!< out-variables
out vec4 fragcolor;


bool radius_test(vec3 position, float radius)
{
return ((position.x*position.x) +(position.y*position.y)+(position.z*position.z)) <= radius;
}

void main()
{
	if (radius_test(passPosition.xyz, 0.3))
	{
		fragcolor = vec4(1,0,0,1);
	}
	else
	{
	fragcolor = vec4(1,1,1,1);
	}
	//fragcolor = vec4(passPosition.x,passPosition.y,passPosition.z,1-passPosition.y);
}