#pragma once

#include <memory>
#include <string>

namespace loquat
{
	class ResourceHandle;

	/// <summary>
	/// Used to load resources from file. There may be several resource loaders,
	/// which do special processing for certian types of files.
	/// </summary>
	class ResourceLoader
	{
	public:
		/// <summary>
		/// Whether we append a null/zero at the end of the data.
		/// </summary>
		/// <returns>If a null gets appended to the end of the data.</returns>
		virtual bool append_null();

		/// <summary>
		/// Whether we discard the raw buffer after loading. This is the case for
		/// things like compressed data that we have to unpack, or transform, 
		/// as we don't want to keep two copies of the data around.
		/// </summary>
		/// <returns>Whether the raw data is dicarded after loading.</returns>
		virtual bool discard_raw_buffer_after_load() = 0;

		/// <summary>
		/// Returns the size of the loaded resource, which might be different from
		/// the size stored in the file.
		/// </summary>
		/// <param name="raw_buffer">The raw data.</param>
		/// <param name="raw_size">The size of the raw data.</param>
		/// <returns>The size of the loaded resource.</returns>
		virtual size_t get_loaded_resource_size(char* raw_buffer,
			size_t raw_size) = 0;

		/// <summary>
		/// Return the wildcard pattern that defines which resources the loader
		/// is used for. For example, "*.ogg" is any ogg file, and "*" is any file
		/// at all.
		/// </summary>
		/// <returns>The pattern that defines which files this is used for.
		/// </returns>
		virtual std::string get_pattern() = 0;

		/// <summary>
		/// Load a resource.
		/// </summary>
		/// <param name="raw_buffer">The raw buffer to load into.</param>
		/// <param name="raw_size">The size of the buffer.</param>
		/// <param name="handle">The handle that we want to load.</param>
		/// <returns>Whether we loaded the resource successfully.</returns>
		virtual bool load_resource(char* raw_buffer, size_t raw_size,
			std::shared_ptr<ResourceHandle> handle) = 0;

		/// <summary>
		/// Whether we can use the bits stored in the raw file, without any
		/// processing.
		/// </summary>
		/// <returns>Whether we can use the raw data without processing.</returns>
		virtual bool use_raw_file() = 0;
	};
}