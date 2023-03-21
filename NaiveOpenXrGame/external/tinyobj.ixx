module;

#include <tiny_obj_loader.h>

export module tinyobj;

export namespace tinyobj
{
	using attrib_t = tinyobj::attrib_t;
	using shape_t = tinyobj::shape_t;
	using material_t = tinyobj::material_t;

	bool inline loadObj(attrib_t* attrib, std::vector<shape_t>* shapes,
		std::vector<material_t>* materials, std::string* warn,
		std::string* err, const char* filename,
		const char* mtl_basedir = NULL, bool triangulate = true,
		bool default_vcols_fallback = true)
	{
		return LoadObj(attrib, shapes, materials, warn, err, filename, mtl_basedir, triangulate, default_vcols_fallback);
	}
}