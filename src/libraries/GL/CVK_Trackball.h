#ifndef __CVK_TRACKBALL_H
#define __CVK_TRACKBALL_H

#include "CVK_Camera.h"
#include "CVK_Perspective.h"


class Trackball : public Camera
{
public:
	Trackball( int width, int height, Projection *projection);
	Trackball( int width, int height);
	~Trackball();

	void update( GLFWwindow* window);
	void setCenter( glm::vec3 *center);
	void setRadius( float radius);
	void setUpvector( glm::vec3 *up);

private:
	glm::vec3 m_cameraPos, m_center, m_up;

	float m_oldX, m_oldY;
	float m_sensitivity;
	float m_theta, m_phi, m_radius;
};

#endif /* __CVK_TRACKBALL_H */
