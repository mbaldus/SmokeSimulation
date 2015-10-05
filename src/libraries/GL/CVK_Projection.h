#ifndef __CVK_PROJECTION_H
#define __CVK_PROJECTION_H

#include <glm/glm.hpp>

class Projection
	{
	public:
		glm::mat4 *getProjMatrix();
		void setProjMatrix( glm::mat4 *projection);

		void getNearFar( float *near, float *far);
		float getNear();
		virtual void updateRatio( float ratio) = 0;

	protected:
		float m_znear, m_zfar;

		glm::mat4 m_perspective;
	};

#endif /* __CVK_PROJECTION_H */
