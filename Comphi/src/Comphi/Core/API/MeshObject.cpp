#include "cphipch.h"
#include "MeshObject.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>

namespace Comphi {

	MeshObject::MeshObject(std::string objFile, std::string textureFile)
	{
		initialize(objFile, textureFile);
	}

	MeshObject::MeshObject(VertexArray& vertices, IndexArray& indices, std::string textureFile)
	{
		initialize(vertices, indices, textureFile);
	}

	void MeshObject::initialize(VertexArray& vertices, IndexArray& indices, std::string textureFile)
	{
		this->vertices = std::make_shared<Vulkan::VertexBuffer>(vertices);
		this->indices = std::make_shared<Vulkan::IndexBuffer>(indices);
		if(!textureFile.empty())
			initTextureSampler(textureFile);
		initUBO();
	}

	void MeshObject::initialize(std::string objFile, std::string textureFile)
	{
		ParseObjFile(objFile);
		if (!textureFile.empty())
			initTextureSampler(textureFile);
		initUBO();
	}

	void MeshObject::ParseObjFile(std::string objFile) {
		this->objFile.reload(objFile);

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFile.c_str())) {
			throw std::runtime_error(warn + err);
		}

		VertexArray vArray;
		IndexArray iArray;
		//Init Mesh Array Objects
		std::unordered_map<Vertex, uint32_t> uniqueVertices{}; //vertex,index

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] //vulkan Flipped TexCoords
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				//Hash Vertex
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vArray.size());
					vArray.push_back(vertex);
				}

				iArray.push_back(uniqueVertices[vertex]);
			}
		}

		//init Objects
		vertices = std::make_shared<Vulkan::VertexBuffer>(vArray);
		indices = std::make_shared<Vulkan::IndexBuffer>(iArray);
	}

	void MeshObject::initTextureSampler(std::string textureFile)
	{
		this->textureFile.reload(textureFile);

		//Init Texture 
		texture = std::make_unique<Vulkan::ImageView>(textureFile);
	}

	void MeshObject::initUBO()
	{
		ubos.clear();
		//Init UBOs
		for (size_t i = 0; i < 2; i++) { //2 << MAX_FRAES_IN_FLIGHT
			UniformBufferObject ubo = {};
			ubos.push_back(std::make_shared<Vulkan::UniformBuffer>(ubo));
		}
	}



}