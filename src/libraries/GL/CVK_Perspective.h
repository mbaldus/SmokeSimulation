#ifndef __CVK_PERSP_H
#define __CVK_PERSP_H

#include "CVK_Projection.h"
#include <glm/gtc/matrix_transform.hpp>

	class Perspective : public Projection
	{
	public:
		Perspective( float ratio);    
		Perspective( float fov, float ratio, float near, float far);    
		~Perspective();

		void setPerspective( float fov, float ratio, float near, float far); 
		void setNearFar( float near, float far);
		void setFov( float fov);
		float getFov();
		void updateRatio( float ratio);

	protected:
		float m_fov, m_ratio;
	};

#endif /* __CVK_PERSP_H */
