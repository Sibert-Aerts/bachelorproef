#ifndef SRC_MAIN_CPP_SIM_WORLDENVIRONMENT_H_
#define SRC_MAIN_CPP_SIM_WORLDENVIRONMENT_H_

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
 *  Copyright 2015, Willem L, Kuylen E, Stijven S & Broeckhove J
 */
/**
 * @file
 * Header file for the WorldEnvironment class.
 */

#include <cstdlib>
#include <memory>
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/property_tree/ptree.hpp>

namespace indismo {

/**
 * Class that keeps track of the 'state' of simulated world.
 * E.g. what day it is, holidays, quarantines, ...
 */
class WorldEnvironment {
public:
	/// Constructor
	WorldEnvironment(const boost::property_tree::ptree& pt_config);

	/// Desctructor
	virtual ~WorldEnvironment();

	/// Advance the internal calendar by one day
	void AdvanceDay();

	/// Get the current day of the simulation
	size_t GetSimulationDay() const;

	/// Get the current day of the month
	size_t GetDay() const;

	/// Get the current month
	size_t GetMonth() const;

	/// Get the current year
	size_t GetYear() const;

	/// Get the current day of the week
	/// 0 (Sunday), ..., 6 (Saturday)
	size_t GetDayOfTheWeek() const;

	/// Check if it's the weekend
	bool IsWeekend() const;

	/// Check if it's a holiday
	bool IsHoliday() const;

	/// Check if it's a school holiday
	bool IsSchoolHoliday() const;

private:
	void InitializeHolidays(std::string holidays_file);

private:
	size_t                                 m_day;                     ///< The current simulation day
	boost::gregorian::date                 m_date;                    ///< The current simulated day

	std::vector<boost::gregorian::date>    m_holidays;                ///< Vector of general holidays
	std::vector<boost::gregorian::date>    m_school_holidays;         ///< Vector of school holidays
};

} // end_of_namespace

#endif // end of include guard
