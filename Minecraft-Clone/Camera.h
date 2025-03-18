#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class Camera 
{
public:
	Camera(glm::vec3 pos, glm::vec3 dir) {
		position = pos;

		dir = glm::normalize(dir);
		forward = dir;

		// if 'forward' and 'up' are nearly colinear, change default up direction
		if (glm::abs(glm::dot(forward, up)) >= 1.f - glm::epsilon<float>()) {
			up = { 0, 1, 0 };
		}

		right = glm::normalize(glm::cross(forward, up));
		pitch = glm::degrees(asin(dir.z));
		yaw = glm::degrees(atan2(dir.y, dir.x));

		calculateView();
	}

	bool update(float deltaSeconds) {
		if (moveInput == glm::vec3(0) && lookInput == glm::vec2(0)) {
			return false;
		}

		// apply move input
		if (moveInput != glm::vec3(0, 0, 0)) {
			moveInput *= moveSpeed;
			
			glm::vec3 localMovement = (moveInput.x * right) +
									  (moveInput.y * forward) +
									  (moveInput.z * up);
			position += localMovement * deltaSeconds;
			
			moveInput = { 0, 0, 0 };
		}

		// apply look input
		if (lookInput != glm::vec2(0, 0)) {
			lookInput *= sensitivity;

			yaw += -lookInput.x;
			pitch += lookInput.y;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.z = sin(glm::radians(pitch));
			forward = glm::normalize(direction);
			right = glm::normalize(glm::cross(forward, up));
			
			lookInput = { 0, 0 };
		}

		calculateView();

		return true;
	}

	// @param move => { right, forward, up }
	void addMoveInput(glm::vec3 move) {
		moveInput += glm::normalize(move);
	}

	// @param look => { yaw, pitch }
	void addLookInput(glm::vec2 look) {
		lookInput += look;
	}

	const glm::mat4& getView() const { return view; }
	const glm::vec3& getPosition() const { return position; }
	const glm::vec3& getForwardDir() const { return forward; }

private:
	void calculateView() {
		view = glm::lookAt(position, position + forward, up);
	}

private:
	glm::vec3 position = { 3, 0, 0 };
	glm::vec3 forward = { -1, 0, 0 };
	glm::vec3 up = { 0, 0, 1 };
	glm::vec3 right = { 0, 0, 0 };

	glm::mat4 view;

	glm::vec3 moveInput = { 0, 0, 0 };
	glm::vec2 lookInput = { 0, 0 };

	float yaw = 180, pitch = 0;

public:
	// config
	float moveSpeed = 5.0f;
	float sensitivity = 0.1f;
};