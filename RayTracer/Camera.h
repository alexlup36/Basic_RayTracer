#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Common.h"

#include "glm/gtx/rotate_vector.hpp"

class Camera
{
public:
	Camera(
		vec3& vPos, 
		float fVerticalFieldOfView,
		float fHorizontalFieldOfView)
		: m_vCameraPosition(vPos), 
		  m_fVerticalFOV(fVerticalFieldOfView),
		  m_fHorizontalFOV(fHorizontalFieldOfView) 
	{
		m_fPitchRotation = 0.0f;
		m_fYawRotation = 0.0f;
	}

	// Getters
	inline const glm::vec3& GetCameraPosition() { return m_vCameraPosition; }
	inline const glm::vec3& GetCameraTarget() { return m_vCameraTarget; }
	inline const glm::vec3& GetCameraUp() { return m_vCameraUp; }

	inline const float GetXRotation() { return m_fYawRotation; }
	inline const float GetYRotation() { return m_fPitchRotation; }
	
	inline float GetVerticalFOV() { return m_fVerticalFOV; }
	inline float GetHorizontalFOV() { return m_fHorizontalFOV; }

	// Setters
	inline void SetPosition(const vec3& pos) { m_vCameraPosition = pos; }
	inline void AddToCameraPosition(const glm::vec3& offset) 
	{ 
		glm::mat4 cameraRotation = glm::mat4(1.0f);
		cameraRotation = glm::rotate(cameraRotation, m_fYawRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		cameraRotation = glm::rotate(cameraRotation, m_fPitchRotation, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::vec3 transformedOffset = glm::vec3(cameraRotation * glm::vec4(offset, 1.0f));
		m_vCameraPosition += transformedOffset;

		UpdateViewMatrix();
	}
	inline void SetTarget(const vec3& target) { m_vCameraTarget = target; } 
	inline void SetUp(const vec3& up) { m_vCameraUp = up; }

	inline void AddXRotation(float dt, float rotation)
	{
		m_fPitchRotation -= m_fRotationSpeed * rotation * dt;
	}

	inline void AddYRotation(float dt, float rotation)
	{
		m_fYawRotation -= m_fRotationSpeed * rotation * dt;
	}

	void UpdateViewMatrix()
	{
		glm::mat4 cameraRotation = glm::mat4(1.0f);
		cameraRotation = glm::rotate(cameraRotation, m_fYawRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		cameraRotation = glm::rotate(cameraRotation, m_fPitchRotation, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::vec4 cameraOriginalTarget = glm::vec4(glm::vec3(0.0f, 0.0f, 3.0f), 1.0f);
		glm::vec4 cameraRotatedTarget = cameraRotation * cameraOriginalTarget;
		m_vCameraTarget = glm::vec3(glm::vec4(m_vCameraPosition, 1.0f) + cameraRotatedTarget);

		glm::vec4 cameraOriginalUp = glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);
		m_vCameraUp = glm::vec3(cameraRotation * cameraOriginalUp);

		m_ViewMatrix = glm::lookAt(m_vCameraPosition, m_vCameraTarget, m_vCameraUp);
	}

	virtual ~Camera(void) {  }

private:
	// Camera information
	vec3 m_vCameraPosition;
	vec3 m_vCameraTarget;
	vec3 m_vCameraUp;
	
	float m_fVerticalFOV;
	float m_fHorizontalFOV;

	float m_fPitchRotation;
	float m_fYawRotation;

	glm::mat4 m_ViewMatrix;

	const float m_fRotationSpeed = 0.03f;
};

#endif // __CAMERA_H__