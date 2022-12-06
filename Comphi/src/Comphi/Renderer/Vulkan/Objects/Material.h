#pragma once

#include "Comphi/Renderer/IMaterial.h"
#include "GraphicsPipeline.h"
#include "ShaderProgram.h"

namespace Comphi::Vulkan {

	class Material : public IMaterial 
	{
	public:
		
		Material(Comphi::MaterialProperties properties);
		~Material();

		//Each Material owns one Shader/Graphics Pipeline
		GraphicsPipeline graphicsPipeline;

		//linked MaterialProperties:
		std::vector<ShaderProgram*> shaderPrograms = std::vector<ShaderProgram*>();

		virtual void bind(void* commandBuffer);
		virtual void bindDescriptorSet(void* commandBuffer, uint32_t currentFrame);
		void sendDescriptorSet(std::vector<UniformBuffer>& MVP_ubos);

	protected:

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		bool BindProgram(IShaderProgram& shaderProgram);
		bool UnbindProgram(IShaderProgram& shaderProgram);

	};

}

