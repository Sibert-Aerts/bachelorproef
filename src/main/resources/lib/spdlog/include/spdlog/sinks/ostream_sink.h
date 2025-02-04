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

#include <memory>
#include <mutex>
#include <ostream>

#include "../details/null_mutex.h"
#include "./base_sink.h"

namespace spdlog {
namespace sinks {
template <class Mutex>
class ostream_sink : public base_sink<Mutex>
{
public:
	explicit ostream_sink(std::ostream& os, bool force_flush = false) : _ostream(os), _force_flush(force_flush) {}
	ostream_sink(const ostream_sink&) = delete;
	ostream_sink& operator=(const ostream_sink&) = delete;
	virtual ~ostream_sink() = default;

protected:
	void _sink_it(const details::log_msg& msg) override
	{
		_ostream.write(msg.formatted.data(), msg.formatted.size());
		if (_force_flush)
			_ostream.flush();
	}

	void flush() override { _ostream.flush(); }

	std::ostream& _ostream;
	bool _force_flush;
};

typedef ostream_sink<std::mutex> ostream_sink_mt;
typedef ostream_sink<details::null_mutex> ostream_sink_st;
}
}
