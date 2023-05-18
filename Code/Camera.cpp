#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

#include "platform.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Init(glm::vec3 pos, float near, float far, float aspRatio)
{
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);

	position = pos;
	nearPlane = near;
	farPlane = far;
	aspectRatio = aspRatio;

	projection = glm::perspective(glm::radians(60.0f), aspectRatio, nearPlane, farPlane);
	view = glm::lookAt(position, position + front, up);
}

void Camera::Update(Input input, f32 dt)
{
	constexpr float speed = 5.0f;

	if (input.keys[K_W] == BUTTON_PRESSED)
	{
		position += front * speed * dt;
	}
	if (input.keys[K_S] == BUTTON_PRESSED)
	{
		position -= front * speed * dt;
	}
	if (input.keys[K_A] == BUTTON_PRESSED)
	{
		position -= glm::normalize(glm::cross(front, up)) * speed * dt;
	}
	if (input.keys[K_D] == BUTTON_PRESSED)
	{
		position += glm::normalize(glm::cross(front, up)) * speed * dt;
	}

	view = glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
}

void Camera::Resize(int width, int height)
{
	aspectRatio = (float)width / (float)height;
	projection = glm::perspective(glm::radians(60.0f), aspectRatio, nearPlane, farPlane);
}