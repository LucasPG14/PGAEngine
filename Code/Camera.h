#pragma once

#include "glm/glm.hpp"

struct Input;

typedef float f32;

class Camera
{
public:
	Camera();
	~Camera();

	void Init(glm::vec3 pos, glm::vec3 t, float near, float far, float aspRatio);
	void Update(Input input, f32 dt);

	const glm::mat4& GetViewMatrix() { return view; }
	const glm::mat4& GetProjectionMatrix() { return projection; }
	const glm::mat4 GetViewProjection() { return projection * view; }

	const glm::vec3& GetPosition() { return position; }

private:
	glm::vec3 position;
	glm::vec3 target;

	glm::mat4 view;
	glm::mat4 projection;

	float aspectRatio;
	float nearPlane;
	float farPlane;
};