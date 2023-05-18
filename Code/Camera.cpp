#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLM/gtx/orthonormalize.hpp>
#include <glm/gtc/quaternion.hpp>

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
	mouseInitialPos = glm::vec2(0.0f);

	projection = glm::perspective(glm::radians(60.0f), aspectRatio, nearPlane, farPlane);
	view = glm::lookAt(position, position + front, up);
}

void Camera::Update(Input input, f32 dt)
{
	constexpr float speed = 5.0f;

	glm::vec3 newPos = position;
	glm::vec3 newFront = front;
	glm::vec3 newUp = up;

	const glm::vec2& mouse = input.mousePos;
	glm::vec2 delta = (mouse - mouseInitialPos) * 0.0001f;
	mouseInitialPos = mouse;

	delta.x = -delta.x;
	delta.y = -delta.y;
	if (input.mouseButtons[RIGHT] == BUTTON_PRESSED)
	{
		if (input.keys[K_W] == BUTTON_PRESSED)
		{
			newPos += front * speed * dt;
		}
		if (input.keys[K_S] == BUTTON_PRESSED)
		{
			newPos -= front * speed * dt;
		}
		if (input.keys[K_A] == BUTTON_PRESSED)
		{
			newPos -= glm::normalize(glm::cross(front, up)) * speed * dt;
		}
		if (input.keys[K_D] == BUTTON_PRESSED)
		{
			newPos += glm::normalize(glm::cross(front, up)) * speed * dt;
		}
		if (input.keys[K_Q] == BUTTON_PRESSED)
		{
			newPos += up * 0.025f;
		}
		if (input.keys[K_E] == BUTTON_PRESSED)
		{
			newPos -= up * 0.025f;
		}

		if (delta.y != 0)
		{
			const glm::quat& quaternion = glm::quat(delta.y * (dt * 1000.0f), glm::normalize(glm::cross(front, up)));
			const glm::quat& conjQuat = glm::conjugate(quaternion);

			newFront = glm::normalize(quaternion * newFront * conjQuat);
			newUp = glm::normalize(quaternion * newUp * conjQuat);
			newFront = glm::orthonormalize(newFront, newUp);
		}
		if (delta.x != 0)
		{
			const glm::quat& quaternion = glm::quat(delta.x * (dt * 1000.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			const glm::quat& conjQuat = glm::conjugate(quaternion);

			newFront = glm::normalize(quaternion * newFront * conjQuat);
			newUp = glm::normalize(quaternion * newUp * conjQuat);
			glm::orthonormalize(newFront, newUp);
		}

		position = newPos;
		up = newUp;
		front = newFront;
	}

	view = glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
}

void Camera::Resize(int width, int height)
{
	aspectRatio = (float)width / (float)height;
	projection = glm::perspective(glm::radians(60.0f), aspectRatio, nearPlane, farPlane);
}