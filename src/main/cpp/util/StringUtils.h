#ifndef INC_STRINGUTILS_H
#define INC_STRINGUTILS_H

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace stride {
namespace util {

/**
 * String utilities.
 */
class StringUtils
{
public:
	/// Builds a value of type T representation from a string.
	template <typename T>
	inline static T FromString(std::string const& s)
	{
		std::stringstream ss(s);
		T t;
		ss >> t;
		return t;
	}

	/// Split a string (in order of occurence) by splitting it on the given delimiters.
	static std::vector<std::string> Split(const std::string& str, const std::string& delimiters)
	{
		std::vector<std::string> tokens;
		boost::algorithm::split(tokens, str, boost::is_any_of(delimiters));
		return tokens;
	}

	/// Tokenize a string (in order of occurence) with the given delimiters.
	/// Multiple consecutive delimiters do NOT define "empty" tokens; they
	/// are simply skipped.
	static std::vector<std::string> Tokenize(const std::string& str, const std::string& delimiters)
	{
		std::vector<std::string> tokens;

		// Skip delimiters at beginning.
		std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		// Find first non-delimiter.
		std::string::size_type pos = str.find_first_of(delimiters, lastPos);

		while (std::string::npos != pos || std::string::npos != lastPos) {
			// Found a token, add it to the vector.
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			// Skip delimiters.
			lastPos = str.find_first_not_of(delimiters, pos);
			// Find next non-delimiter.
			pos = str.find_first_of(delimiters, lastPos);
		}

		return tokens;
	}

	/// Builds a string representation of a value of type T.
	template <typename T>
	inline static std::string ToString(T const& value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	/// Builds a string representation with minimum width of a value of type T.
	template <typename T>
	inline static std::string ToString(T const& value, int width, char fill = ' ')
	{
		std::stringstream ss;
		ss << std::setw(width) << std::setfill(fill) << value;
		return ss.str();
	}

	/// Builds a string with lower case characters only.
	static std::string ToLower(std::string const& source)
	{
		auto lower = [](int c) -> int { return std::toupper(c); };
		std::string copy;
		std::transform(source.begin(), source.end(), std::back_inserter(copy), lower);
		return copy;
	}

	/// Builds a string with upper case characters only.
	static std::string ToUpper(std::string const& source)
	{
		auto upper = [](int c) -> int { return std::toupper(c); };
		std::string copy;
		std::transform(source.begin(), source.end(), std::back_inserter(copy), upper);
		return copy;
	}

	/// Trim characters at right end of string.
	static std::string TrimRight(std::string const& source, std::string const& t = " ")
	{
		std::string str = source;
		return str.erase(str.find_last_not_of(t) + 1);
	}

	/// Trim characters at left end of string.
	static std::string TrimLeft(std::string const& source, std::string const& t = " ")
	{
		std::string str = source;
		return str.erase(0, source.find_first_not_of(t));
	}

	/// Trim characters at both ends of string.
	static std::string Trim(std::string const& source, std::string const& t = " ")
	{
		std::string str = source;
		return TrimLeft(TrimRight(str, t), t);
	}
};

} // end namespace
} // end namespace

#endif // end-of-include-guard
