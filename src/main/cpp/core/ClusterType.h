#ifndef CLUSTER_TYPE_H_INCLUDED
#define CLUSTER_TYPE_H_INCLUDED
/*
 *  This is free software: you can redistribute it and/or modify it
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
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */

/**
 * @file
 * Definition of ClusterType.
 */

#include <string>

namespace indismo {

/// Enumerates the cluster types.
enum class ClusterType
{
	Household, School, Work, HomeDistrict, DayDistrict, Null
};

/// Converts a ClusterType value to corresponding name.
std::string ToString(ClusterType w);

/// Check whether string is name of a ClusterType value.
bool IsClusterType(const std::string& s);

/// Converts a string with name to ClusterType value.
ClusterType ToClusterType(const std::string& s);

} // namespace

#endif // end-of-include-guard
