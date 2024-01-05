#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace loquat
{
	/// <summary>
	/// A single shader module, representing one file.
	/// </summary>
	struct ShaderModule
	{
		ShaderModule(const std::string& name);
		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;
		ShaderModule(ShaderModule&&) = delete;
		ShaderModule& operator=(ShaderModule&&) = delete;
		~ShaderModule();

		VkShaderModule shader_module = nullptr;
	};

	/// <summary>
	/// The types of shaders we support.
	/// </summary>
	enum class ShaderType
	{
		fragment,
		vertex
	};

	/// <summary>
	/// Convert our shader type to the type that we need to create a shader
	/// of that type in Vulkan.
	/// </summary>
	/// <param name="stage">The type of shader.</param>
	/// <returns>The corresponding stage flags for that stage.</returns>
	[[nodiscard]]
	constexpr VkShaderStageFlagBits stage_type(ShaderType stage) noexcept;

	/// <summary>
	/// Information used to specify which modules are required for a shader.
	/// </summary>
	struct ShaderStage
	{
		const ShaderType type;
		const std::string location;

		constexpr ShaderStage(const ShaderType type,
			const std::string& location) noexcept
			: type{ type }
			, location{ location }
		{}
		constexpr ShaderStage(const ShaderStage&) = default;
		ShaderStage& operator=(const ShaderStage&) = default;
		constexpr ShaderStage(ShaderStage&&) = default;
		ShaderStage& operator=(ShaderStage&&) = default;
		~ShaderStage() = default;
	};

	/// <summary>
	/// A shader program defined by one or more stages.
	/// </summary>
	struct Shader
	{
	public:
		Shader(std::initializer_list<ShaderStage> stages) noexcept;
		~Shader();
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;

		std::vector<std::shared_ptr<ShaderModule>> modules;
		std::vector<VkPipelineShaderStageCreateInfo> create_info_list;
	};
}