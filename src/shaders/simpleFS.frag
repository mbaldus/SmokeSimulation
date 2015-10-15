#version 330

//!< in-variables
in vec4 passPosition;

//!< out-variables
out vec4 fragcolor;

void main()
{
	fragcolor = vec4(passPosition.x,passPosition.y,passPosition.z,1-passPosition.y);
}