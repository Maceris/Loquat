#pragma once

#include <source_location>
#include <string>
#include <string_view>

#include "main/memory_utils.h"

using LogFlag = unsigned char;

/// <summary>
/// A no-op flag that writes nowhere, so we can specify logging more 
/// consistently. Not intended to be combined with other flags.
/// </summary>
const LogFlag FLAG_WRITE_NOWHERE = 0;

/// <summary>
/// Writes the log to a file.
/// </summary>
const LogFlag FLAG_WRITE_TO_LOG_FILE = 1 << 0;

/// <summary>
/// Writes the log to the debugger.
/// </summary>
const LogFlag FLAG_WRITE_TO_DEBUGGER = 1 << 1;

namespace Logger
{
	/// <summary>
	/// Just used by macros, shouldn't be referenced elsewhere.
	/// </summary>
	class ErrorLogger
	{
		/// <summary>
		/// Used to allow the logger to log once, and then disable itself.
		/// </summary>
		bool enabled;
	public:
		/// <summary>
		/// Set up a new logger.
		/// </summary>
		ErrorLogger();

		/// <summary>
		/// Record a log.
		/// </summary>
		/// <param name="error_message">The message to log.</param>
		/// <param name="fatal">Whether the message is fatal.</param>
		/// <param name="location">The location of the log line.</param>
		void log_error(std::string_view error_message, bool fatal,
			std::source_location location);
	};

	/// <summary>
	/// Set up the logger, must be called at the beginning of the program.
	/// </summary>
	void init();

	/// <summary>
	/// Destroy the program should be called at the end of the program.
	/// </summary>
	void destroy();

	/// <summary>
	/// Record a log without any location.
	/// </summary>
	/// <param name="tag">The tag we are logging.</param>
	/// <param name="message">The message to log.</param>
	void log(std::string_view tag, std::string_view message);

	/// <summary>
	/// Record a log.
	/// </summary>
	/// <param name="tag">The tag we are logging.</param>
	/// <param name="error_message">The message to log.</param>
	/// <param name="location">The location of the log line.</param>
	void log(std::string_view tag, std::string_view error_message,
		std::source_location location);

	/// <summary>
	/// Set up display flags for any particular flag, so tags can be used
	/// when logging.
	/// </summary>
	/// <param name="tag">The tag to configure.</param>
	/// <param name="flags">Flags for this tag.</param>
	void set_display_flags(std::string_view tag, unsigned char flags);
}

/*
These are wrapped in do {} while (0) so they can be used like function calls
in all contexts, like after conditionals.
*/ 

/// <summary>
/// Log a fatal error, these are always shown to the user.
/// </summary>
#define LOG_FATAL(str) \
	do \
	{ \
		static Logger::ErrorLogger* error_logger = loquat::alloc<Logger::ErrorLogger>(); \
		std::string s((str)); \
		error_logger->log_error(s, true, std::source_location::current()); \
	} \
	while (0)\

#if _DEBUG

	/// <summary>
	/// Log an error that is potentially fatal, ask the user what to do about it.
	/// Ignored in release mode.
	/// </summary>
	#define LOG_ERROR(str) \
		do \
		{ \
			static Logger::ErrorLogger* error_logger = loquat::alloc<Logger::ErrorLogger>(); \
			std::string s((str)); \
			error_logger->log_error(s, false, std::source_location::current()); \
		} \
		while (0)\

	/// <summary>
	/// Log a warning, which is recoverable.
	/// </summary>
	#define LOG_WARNING(str) \
		do \
		{ \
			std::string s((str)); \
			Logger::log("WARNING", s, std::source_location::current()); \
		}\
		while (0)\

	/// <summary>
	/// Log some information without any kind of tags (technically has a tag 
	/// of "INFO").
	/// </summary>
	#define LOG_INFO(str) \
		do \
		{ \
			std::string s((str)); \
			Logger::log("INFO", s); \
		} \
		while (0) \

	/// <summary>
	/// Used for logging any desired tag string, but tags should be enabled during
	/// initialization or their logs won't go anywhere.
	/// </summary>
	#define LOG_TAGGED(tag, str) \
		do \
		{ \
			std::string s((str)); \
			Logger::log(tag, s); \
		} \
		while (0) \

	/// <summary>
	/// Asserts that an expression is true, and if it is not then we log the
	/// issue.
	/// </summary>
	#define LOG_ASSERT(expr) \
		do \
		{ \
			if (!(expr)) \
			{ \
				static Logger::ErrorLogger* error_logger = loquat::alloc<Logger::ErrorLogger>(); \
				error_logger->log_error(#expr, false, std::source_location::current()); \
			} \
		} \
		while (0) \

#else

	// Release mode definitions for the macros above that are completely ignored
	// by the complier.

	/// <summary>
	/// Does nothing in release mode.
	/// </summary>
	#define LOG_ERROR(str) do { (void)sizeof(str); } while(0) 

	/// <summary>
	/// Does nothing in release mode.
	/// </summary>
	#define LOG_WARNING(str) do { (void)sizeof(str); } while(0) 

	/// <summary>
	/// Does nothing in release mode.
	/// </summary>
	#define LOG_INFO(str) do { (void)sizeof(str); } while(0) 

	/// <summary>
	/// Does nothing in release mode.
	/// </summary>
	#define LOG_TAGGED(tag, str) do { (void)sizeof(tag); (void)sizeof(str); } while(0) 

	/// <summary>
	/// Does nothing in release mode.
	/// </summary>
	#define LOG_ASSERT(expr) do { (void)sizeof(expr); } while(0) 

#endif
