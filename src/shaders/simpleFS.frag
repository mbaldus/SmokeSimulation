#version 330

//!< in-variables
in vec4 passPosition;

//!< out-variables
out vec4 fragcolor;

void main()
{
	fragcolor = vec4(1,1,1,1-passPosition.y);
}