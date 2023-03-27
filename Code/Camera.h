#pragma once

#include "glm/glm.hpp"

class Camera
{
public:
	Camera();
	~Camera();

	void Init(glm::vec3 pos, glm::vec3 t, float near, float far, float aspRatio);
	void Update();

	const glm::mat4& GetViewMatrix() { return view; }
	const glm::mat4& GetProjectionMatrix() { return projection; }
	const glm::mat4 GetViewProjection() { return projection * view; }

private:
	glm::vec3 position;
	glm::vec3 target;

	glm::mat4 view;
	glm::mat4 projection;

	float aspectRatio;
	float nearPlane;
	float farPlane;
};