#pragma once

#include <string>

namespace loquat
{
	class Resource;

	/// <summary>
	/// A file that can be opened and closed, and provides the application with
	/// resources.
	/// </summary>
	class ResourceFile {
	public:
		/// <summary>
		/// Open the file and return success or failure.
		/// </summary>
		/// <returns>Whether we opened the file without error.</returns>
		virtual bool open() = 0;

		/// <summary>
		/// Fetch the size of a resource based on the name. If the resource
		/// is not found, returns -1.
		/// </summary>
		/// <param name="resource">The resource we want the size of.</param>
		/// <returns>The size, in bytes.</returns>
		virtual size_t get_raw_resource_size(const Resource& resource) = 0;

		/// <summary>
		/// Read the resource from the file.
		/// </summary>
		/// <param name="resource">The resource to read.</param>
		/// <param name="buffer">The buffer to load into.</param>
		/// <returns>The size of the loaded resource, in bytes.</returns>
		virtual size_t load_resource(const Resource& resource, char* buffer) = 0;

		/// <summary>
		/// Calculates the number of resources that are in the file.
		/// </summary>
		/// <returns>The number of resources in the file.</returns>
		virtual const size_t get_resource_count() = 0;

		/// <summary>
		/// Fetches the name of the n'th resource in the file.
		/// </summary>
		/// <param name="index">The index of the resource.</param>
		/// <returns>The name of the resource at the supplied index.
		/// Empty if index is not in bounds.
		/// </returns>
		virtual std::string get_resource_name(size_t index) = 0;

		/// <summary>
		/// Clean up.
		/// </summary>
		virtual ~ResourceFile() { }
	};
}