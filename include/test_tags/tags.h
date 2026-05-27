#pragma once

#include <glm/glm.hpp>

namespace test {
	struct Position {
		using type = glm::vec3;

		static type defaultValue() {
			return glm::vec3(0.f, 0.f, 0.f);
		}
	};
	struct Color {
		using type = glm::vec4;

		static type defaultValue() {
			return glm::vec4(0.f, 0.f, 0.f, 0.f);
		}
	};
	struct TexCoords {
		using type = glm::vec2;

		static type defaultValue() {
			return glm::vec2(0.f, 0.f);
		}
	};
}