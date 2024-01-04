#pragma once

#include <string>

namespace loquat
{
	/// <summary>
	/// Extra data associated with a resource. For example, length and format
	/// of a sound file, or a parsed version of a resource requiring extra
	/// processing.
	/// </summary>
	class ResourceExtraData
	{
	public:
		/// <summary>
		/// Return a string indicating the type of data.
		/// </summary>
		/// <returns>The type of data.</returns>
		virtual std::string to_string() = 0;
	};

	/// <summary>
	/// A unique identifier of a resource.
	/// </summary>
	class Resource
	{
	public:
		/// <summary>
		/// The name of the resource, typically including the path to the
		/// resource.
		/// </summary>
		std::string name;

		/// <summary>
		/// Create a new resource.
		/// </summary>
		/// <param name="resource_name">The name of the resource, typically
		/// including the path to the resource.</param>
		Resource(const std::string& resource_name);
	};
}