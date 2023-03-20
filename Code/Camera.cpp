#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Init(glm::vec3 pos, glm::vec3 t, float near, float far, float aspRatio)
{
	position = pos;
	target = t;
	nearPlane = near;
	farPlane = far;
	aspectRatio = aspRatio;

	projection = glm::perspective(glm::radians(60.0f), aspectRatio, nearPlane, farPlane);
	view = glm::lookAt(pos, target, glm::vec3(0, 1, 0));
}

void Camera::Update()
{
}