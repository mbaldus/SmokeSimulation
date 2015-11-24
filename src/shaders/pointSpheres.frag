#version 330

//!< in-variables
in vec4 passPosition;

//!< uniforms
uniform vec3 lightDir;
uniform sampler2D tex;

//!< out-variables
out vec4 fragcolor;


void main()
{
//calculate normal from texture coordinates
	vec3 normal;
	normal.xy= gl_PointCoord * 2.0 - vec2(1.0);
	float mag = dot(normal.xy,normal.xy);
	if (mag > 1.0) discard;
	normal.z = sqrt(1.0-mag);

	float diffuse=max(0.0,dot(lightDir, normal));

	fragcolor = vec4(1,1,0,1) * diffuse;
}