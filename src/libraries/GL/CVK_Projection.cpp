#include "CVK_Projection.h"

glm::mat4 *Projection::getProjMatrix( )
{
	return &m_perspective;
	
}
void Projection::setProjMatrix( glm::mat4 *projection)
{
	m_perspective = *projection;
}

void Projection::getNearFar( float *near, float *far)
{
	*near = m_znear;
	*far = m_zfar;
}

float Projection::getNear()
{
	return m_znear;
}