/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

//
// Global registry functions
//
#include "../sinks/file_sinks.h"
#include "../sinks/stdout_sinks.h"
#include "../sinks/syslog_sink.h"
#include "registry.h"

inline void spdlog::register_logger(std::shared_ptr<logger> logger)
{
	return details::registry::instance().register_logger(logger);
}

inline std::shared_ptr<spdlog::logger> spdlog::get(const std::string& name)
{
	return details::registry::instance().get(name);
}

inline void spdlog::drop(const std::string& name) { details::registry::instance().drop(name); }

// Create multi/single threaded rotating file logger
inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_mt(
    const std::string& logger_name, const std::string& filename, size_t max_file_size, size_t max_files,
    bool force_flush)
{
	return create<spdlog::sinks::rotating_file_sink_mt>(
	    logger_name, filename, "txt", max_file_size, max_files, force_flush);
}

inline std::shared_ptr<spdlog::logger> spdlog::rotating_logger_st(
    const std::string& logger_name, const std::string& filename, size_t max_file_size, size_t max_files,
    bool force_flush)
{
	return create<spdlog::sinks::rotating_file_sink_st>(
	    logger_name, filename, "txt", max_file_size, max_files, force_flush);
}

// Create file logger which creates new file at midnight):
inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_mt(
    const std::string& logger_name, const std::string& filename, int hour, int minute, bool force_flush)
{
	return create<spdlog::sinks::daily_file_sink_mt>(logger_name, filename, "txt", hour, minute, force_flush);
}
inline std::shared_ptr<spdlog::logger> spdlog::daily_logger_st(
    const std::string& logger_name, const std::string& filename, int hour, int minute, bool force_flush)
{
	return create<spdlog::sinks::daily_file_sink_st>(logger_name, filename, "txt", hour, minute, force_flush);
}

// Create stdout/stderr loggers
inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_mt(const std::string& logger_name)
{
	return details::registry::instance().create(logger_name, spdlog::sinks::stdout_sink_mt::instance());
}

inline std::shared_ptr<spdlog::logger> spdlog::stdout_logger_st(const std::string& logger_name)
{
	return details::registry::instance().create(logger_name, spdlog::sinks::stdout_sink_st::instance());
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_mt(const std::string& logger_name)
{
	return details::registry::instance().create(logger_name, spdlog::sinks::stderr_sink_mt::instance());
}

inline std::shared_ptr<spdlog::logger> spdlog::stderr_logger_st(const std::string& logger_name)
{
	return details::registry::instance().create(logger_name, spdlog::sinks::stderr_sink_st::instance());
}

#ifdef __linux__
// Create syslog logger
inline std::shared_ptr<spdlog::logger> spdlog::syslog_logger(
    const std::string& logger_name, const std::string& syslog_ident, int syslog_option)
{
	return create<spdlog::sinks::syslog_sink>(logger_name, syslog_ident, syslog_option);
}
#endif

// Create logger with multiple sinks

inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, spdlog::sinks_init_list sinks)
{
	return details::registry::instance().create(logger_name, sinks);
}

template <typename Sink, typename... Args>
inline std::shared_ptr<spdlog::logger> spdlog::create(const std::string& logger_name, const Args&... args)
{
	sink_ptr sink = std::make_shared<Sink>(args...);
	return details::registry::instance().create(logger_name, {sink});
}

template <class It>
inline std::shared_ptr<spdlog::logger> spdlog::create(
    const std::string& logger_name, const It& sinks_begin, const It& sinks_end)
{
	return details::registry::instance().create(logger_name, sinks_begin, sinks_end);
}

inline void spdlog::set_formatter(spdlog::formatter_ptr f) { details::registry::instance().formatter(f); }

inline void spdlog::set_pattern(const std::string& format_string)
{
	return details::registry::instance().set_pattern(format_string);
}

inline void spdlog::set_level(level::level_enum log_level)
{
	return details::registry::instance().set_level(log_level);
}

inline void spdlog::set_async_mode(
    size_t queue_size, const async_overflow_policy overflow_policy, const std::function<void()>& worker_warmup_cb,
    const std::chrono::milliseconds& flush_interval_ms)
{
	details::registry::instance().set_async_mode(queue_size, overflow_policy, worker_warmup_cb, flush_interval_ms);
}

inline void spdlog::set_sync_mode() { details::registry::instance().set_sync_mode(); }

inline void spdlog::drop_all() { details::registry::instance().drop_all(); }
