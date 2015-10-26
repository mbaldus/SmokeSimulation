#version 330

//!< in-variables
in vec4 passPosition;

//!< uniforms

//!< out-variables
out vec4 fragcolor;


void main()
{
		fragcolor = vec4(1,0,0,1);
		//fragcolor = vec4(passPosition.x,passPosition.y,passPosition.z,1-passPosition.y);
}