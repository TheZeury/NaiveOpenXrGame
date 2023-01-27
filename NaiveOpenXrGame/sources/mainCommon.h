#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <set>
#include <fstream>
#include <exception>
#include <unordered_map>

#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.hpp>
#include "..\external\xr_linear.h"
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace stb
{
#include "..\external\stb_image.h"
}
#include "..\external\tiny_obj_loader.h"
#if !defined(NDEBUG)
#define LOG_STEP(INSTANCE, STEP) LogStep(INSTANCE, STEP)
#define LOG_SUCCESS() LogSuccess()
#define LOG_FAILURE() LogFailure()
#define LOG_INFO(INSTANCE, INFO, TAB) LogInformation(INSTANCE, INFO, TAB)
#define LOG_ERRO(ERR) LogError(ERR)
#else
#define LOG_STEP(INSTANCE, STEP) 
#define LOG_SUCCESS()
#define LOG_FAILURE()
#define LOG_INFO(INSTANCE, INFO, TAB)
#define LOG_ERRO(ERR)
#endif

#define MIRROR_WINDOW

void inline LogStep(std::string instance, std::string step)
{
	std::cout << "\033[0m[\033[1;36mSTEP\033[0m] " << "\033[1;34m" + instance + "\t\033[0m" + step + "...\t";
}

void inline LogSuccess()
{
	std::cout << "\033[1;32mSuccess\033[0m\n";
}

void inline LogFailure()
{
	std::cout << "\033[1;31mFailure\033[0m\n";
}

void inline LogInformation(std::string instance, std::string information, size_t table)
{
	std::cout << "\033[0m[\033[1;33mINFO\033[0m] " << "\033[1;34m" + instance;
	while (table--) std::cout << '\t';
	std::cout << "\t\033[0m" + information << std::endl;
}

void inline LogError(std::string errorMessage)
{
	std::cout << "\033[0m[\033[1;31mERRO\033[0m] " << errorMessage << std::endl;
}