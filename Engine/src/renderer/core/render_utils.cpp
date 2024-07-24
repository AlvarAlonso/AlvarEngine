#include "render_utils.hpp"
#include <core/logger.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

bool renderutils::LoadMeshFromFile(const std::string& aFilename, sMeshData& aOutMesh)
{
	const auto& FoundMesh = sMeshData::HasMeshData(aFilename);
	if (FoundMesh)
	{
		SGSINFO("The mesh with path %s has already been loaded. Canceling the loading.", aFilename.c_str());
		return false;
	}

	tinyobj::attrib_t Attrib;

	std::vector<tinyobj::shape_t> Shapes;

	std::vector<tinyobj::material_t> Materials;

	std::string Warning;
	std::string Error;

	tinyobj::LoadObj(&Attrib, &Shapes, &Materials, &Warning, &Error, aFilename.c_str());

	if (!Warning.empty())
	{
		SGSWARN(Warning.c_str());
	}

	if (!Error.empty())
	{
		SGSERROR(Error.c_str());
		return false;
	}

	std::unordered_map<sVertex, uint32_t> UniqueVertices{};

	for (const auto& Shape : Shapes)
	{
		for (const auto& Index : Shape.mesh.indices)
		{
			sVertex Vertex{};

			Vertex.Position = 
			{
				Attrib.vertices[3 * Index.vertex_index + 0],
				Attrib.vertices[3 * Index.vertex_index + 1],
				Attrib.vertices[3 * Index.vertex_index + 2]
			};

			Vertex.Normal = 
			{
                Attrib.normals[3 * Index.normal_index + 0],
                Attrib.normals[3 * Index.normal_index + 1],
                Attrib.normals[3 * Index.normal_index + 2]
            };

			Vertex.UV = 
			{
				Attrib.texcoords[2 * Index.texcoord_index + 0],
				1.0f - Attrib.texcoords[2 * Index.texcoord_index + 1]
			};

			Vertex.Color = { 1.0f, 1.0f, 1.0f };

			if (UniqueVertices.count(Vertex) == 0)
			{
				UniqueVertices[Vertex] = static_cast<uint32_t>(aOutMesh.Vertices.size());
				aOutMesh.Vertices.push_back(Vertex);
			}

			aOutMesh.Indices32.push_back(UniqueVertices[Vertex]);
		}
	}

	return true;
}
