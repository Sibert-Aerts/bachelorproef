#ifndef INC_STRINGUTILS_H
#define INC_STRINGUTILS_H
/*
 *  This file is part of the indismo software.
 *  It is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *  The software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  Reference: Willem L, Stijven S, Tijskens E, Beutels P, Hens N and
 *  Broeckhove J. (2015) Optimizing agent-based transmission models for
 *  infectious diseases, BMC Bioinformatics.
 *
 *  Copyright 2015, Willem L, Stijven S & Broeckhove J
 */
/**
 * @file.
 * Conversion from or to string.
 */

#include <string>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace indismo {
namespace util {

/**
 * String utilities.
 */
class StringUtils
{
public:
	/// Builds a value of type T representation from a string.
	template<typename T> inline static T FromString(std::string const& s)
	{
		std::stringstream ss(s);
		T t;
		ss >> t;
		return t;
	}

	/// Tokenize a string (in order of occurence) by splitting it on the given delimiters.
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
	template<typename T> inline static std::string ToString(T const& value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	/// Builds a string representation with minimum width of a value of type T.
	template<typename T> inline static std::string ToString(T const& value, int width, char fill = ' ')
	{
		std::stringstream ss;
		ss << std::setw(width) << std::setfill(fill) << value;
		return ss.str();
	}

	/// Builds a string with upper case characters only.
	static std::string ToUpper(std::string const& source)
	{
		auto upper = [](int c)->int {return std::toupper(c);};
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

} // end namespace output
} // end namespace indismo

#endif // end-of-include-guard
