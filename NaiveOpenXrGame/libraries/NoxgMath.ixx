module;

#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#include "..\external\xr_linear.h"
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
#include <PxPhysicsAPI.h>
using namespace physx;
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

export module NoxgMath;

namespace Noxg
{
	/*export struct vec2
	{
		union { float x, r; };

		union { float y, g; };

		vec2(float x, float y) : x{ x }, y{ y } { }

		vec2(const glm::vec2& other) : x{ other.x }, y{ other.y } { }

		vec2(const PxVec2& other) : x{ other.x }, y{ other.y } { }

		operator glm::vec2& ()
		{
			return *(glm::vec2*)(this);
		}

		operator PxVec2& ()
		{
			return *(PxVec2*)(this);
		}

		glm::vec2* glmPointer()
		{
			return (glm::vec2*)(this);
		}

		PxVec2* physxPointer()
		{
			return (PxVec2*)(this);
		}
	};

	export struct vec3
	{
		union { float x, r; };

		union { float y, g; };

		union { float z, b; };

		vec3(float x, float y, float z) : x{ x }, y{ y }, z{ z } { }

		vec3(const glm::vec3& other) : x{ other.x }, y{ other.y }, z{ other.z } { }

		vec3(const PxVec3& other) : x{ other.x }, y{ other.y }, z{ other.z } { }

		operator vec2 () const
		{
			return vec2{ x, y };
		}

		operator glm::vec2 () const
		{
			return glm::vec2{ x, y };
		}

		operator PxVec2 () const
		{
			return PxVec2{ x, y };
		}

		operator glm::vec3& ()
		{
			return *(glm::vec3*)(this);
		}

		operator PxVec3& ()
		{
			return *(PxVec3*)(this);
		}

		glm::vec3* glmPointer()
		{
			return (glm::vec3*)(this);
		}

		PxVec3* physxPointer()
		{
			return (PxVec3*)(this);
		}
	};

	export struct vec4
	{
		union { float x, r; };

		union { float y, g; };

		union { float z, b; };

		union { float w, a; };

		vec4(float x, float y, float z, float w) : x{ x }, y{ y }, z{ z }, w{ w } { }

		vec4(const glm::vec4& other) : x{ other.x }, y{ other.y }, z{ other.z }, w{ other.w } { }

		vec4(const PxVec4& other) : x{ other.x }, y{ other.y }, z{ other.z }, w{ other.w } { }

		operator vec2 () const
		{
			return vec2{ x, y };
		}

		operator glm::vec2() const
		{
			return glm::vec2{ x, y };
		}

		operator PxVec2 () const
		{
			return PxVec2{ x, y };
		}

		operator vec3 () const
		{
			return vec3{ x, y, z };
		}

		operator glm::vec3() const
		{
			return glm::vec3{ x, y, z };
		}

		operator PxVec3 () const
		{
			return PxVec3{ x, y, z };
		}

		operator glm::vec4& ()
		{
			return *(glm::vec4*)(this);
		}

		operator PxVec4& ()
		{
			return *(PxVec4*)(this);
		}

		glm::vec4* glmPointer()
		{
			return (glm::vec4*)(this);
		}

		PxVec4* physxPointer()
		{
			return (PxVec4*)(this);
		}
	};*/

	export struct vec2 : public glm::vec2
	{
		template<typename... Types>
		vec2(Types... args) : glm::vec2{ args } { }

		vec2(const PxVec2& other) : glm::vec2{ *(glm::vec2*)(&other) } { }

		vec2(const xr::Vector2f& other) : glm::vec2{ *(glm::vec2*)(&other) } { }

		operator PxVec2& ()
		{
			return *(PxVec2*)this;
		}

		operator xr::Vector2f& ()
		{
			return *(xr::Vector2f*)this;
		}
	};

	export struct vec3 : public glm::vec3
	{
		template<typename... Types>
		vec3(Types... args) : glm::vec3{ args } { }

		vec3(const PxVec3& other) : glm::vec3{ *(glm::vec3*)(&other) } { }

		vec3(const xr::Vector3f& other) : glm::vec3{ *(glm::vec3*)(&other) } { }

		operator PxVec3& ()
		{
			return *(PxVec3*)this;
		}

		operator xr::Vector3f& ()
		{
			return *(xr::Vector3f*)this;
		}
	};

	export struct vec4 : public glm::vec4
	{
		template<typename... Types>
		vec4(Types... args) : glm::vec4{ args } { }

		vec4(const PxVec4& other) : glm::vec4{ *(glm::vec4*)(&other) } { }

		vec4(const xr::Vector4f& other) : glm::vec4{ *(glm::vec4*)(&other) } { }

		operator PxVec4& ()
		{
			return *(PxVec4*)this;
		}

		operator xr::Vector4f& ()
		{
			return *(xr::Vector4f*)this;
		}
	};

	export struct mat3 : public glm::mat3
	{
		template<typename... Types>
		mat3(Types... args) : glm::mat3{ args } { }

		mat3(const PxMat33& other) : glm::mat3{ *(glm::mat3*)(&other) } { }

		mat3(const XrMatrix4x4f& other) : glm::mat3{ *(glm::mat4*)(&other) } { }

		operator PxMat33& ()
		{
			return *(PxMat33*)this;
		}
	};

	export struct mat4 : public glm::mat4
	{
		template<typename... Types>
		mat4(Types... args) : glm::mat4{ args } { }

		mat4(const PxMat44& other) : glm::mat4{ *(glm::mat4*)(&other) } { }

		mat4(const XrMatrix4x4f& other) : glm::mat4{ *(glm::mat4*)(&other) } { }

		operator PxMat44& ()
		{
			return *(PxMat44*)this;
		}

		operator XrMatrix4x4f& ()
		{
			return *(XrMatrix4x4f*)this;
		}
	};
}