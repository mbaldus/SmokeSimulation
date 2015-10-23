#version 330

//!< in-variables
in vec4 passPosition;

//!< out-variables
out vec4 fragcolor;

void main()
{
	

	if ((-0.5 < passPosition.x && passPosition.x < 0.5) && (-0.5 < passPosition.y && passPosition.y < 0.5) && (-0.5 < passPosition.z && passPosition.z < 0.5))
	{
		fragcolor = vec4(1,0,0,1);
	}
	else
	{
	fragcolor = vec4(1,1,1,1);
	}
	//fragcolor = vec4(passPosition.x,passPosition.y,passPosition.z,1-passPosition.y);
}