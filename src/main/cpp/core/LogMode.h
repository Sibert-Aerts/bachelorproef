#ifndef LOGMODE_H_INCLUDED
#define LOGMODE_H_INCLUDED
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
 * Header for the core Cluster class.
 */

namespace indismo {
namespace core {

/**
* Enum specifiying the level of logging required:
* \li none at all
* \li only contacts where transimission occurs
* \li all contacts.
*/
enum class LogMode {None = 0U, Transmissions = 1U, Contacts = 2U};


} // end_of_namespace
} // end_of_namespace

#endif // include-guard
