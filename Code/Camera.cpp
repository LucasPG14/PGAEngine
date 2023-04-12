#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

#include "platform.h"

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
	view = glm::lookAt(position, target, glm::vec3(0, 1, 0));
}

void Camera::Update(Input input, f32 dt)
{
	constexpr float speed = 2.0f;

	if (input.keys[K_W] == BUTTON_PRESSED)
	{
		position.z -= 5.0f * dt;
	}
	if (input.keys[K_S] == BUTTON_PRESSED)
	{
		position.z += 5.0f * dt;
	}
	if (input.keys[K_A] == BUTTON_PRESSED)
	{
		position.x -= 5.0f * dt;
	}
	if (input.keys[K_D] == BUTTON_PRESSED)
	{
		position.x += 5.0f * dt;
	}

	view = glm::lookAt(position, target, glm::vec3(0, 1, 0));
}