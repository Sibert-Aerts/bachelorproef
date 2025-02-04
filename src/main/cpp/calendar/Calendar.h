#ifndef SRC_MAIN_CPP_CALENDAR_CALENDAR_H_
#define SRC_MAIN_CPP_CALENDAR_CALENDAR_H_

#include <boost/property_tree/ptree.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>

namespace stride {

/**
 * Class that keeps track of the 'state' of simulated world.
 * E.g. what day it is, holidays, quarantines, ...
 */
class Calendar
{
public:
	Calendar() : m_day(0) {}

	/// Advance the internal calendar by one day
	void AdvanceDay();

	boost::gregorian::date GetDate() const { return m_date; }

	/// Get the current day of the month
	std::size_t GetDay() const { return m_date.day(); }

	/// Get the current day of the week (0 (Sunday), ..., 6 (Saturday))
	std::size_t GetDayOfTheWeek() const { return m_date.day_of_week(); }

	/// Get the current month
	std::size_t GetMonth() const { return m_date.month(); }

	/// Get the current day of the simulation
	std::size_t GetSimulationDay() const { return m_day; }

	/// Get the current year
	std::size_t GetYear() const { return m_date.year(); }

	/// Check if it's a holiday
	bool IsHoliday() const { return (std::find(m_holidays.begin(), m_holidays.end(), m_date) != m_holidays.end()); }

	/// Check if it's a school holiday
	bool IsSchoolHoliday() const
	{
		return (
		    std::find(m_school_holidays.begin(), m_school_holidays.end(), m_date) != m_school_holidays.end());
	}

	/// Check if it's the weekend
	bool IsWeekend() const { return (GetDayOfTheWeek() == 6 || GetDayOfTheWeek() == 0); }

	/// Initializes this calendar from the given start day and holiday ptree.
	void Initialize(const boost::gregorian::date& start_date, const boost::property_tree::ptree& holidays_ptree);

	/// Initializes this calendar from the given start day and holiday file.
	void Initialize(const boost::gregorian::date& start_date, const std::string& holidays_file);

private:
	/// The current simulation day
	std::size_t m_day;

	/// The current simulated day
	boost::gregorian::date m_date;

	/// Vector of general holidays
	std::vector<boost::gregorian::date> m_holidays;

	/// Vector of school holidays
	std::vector<boost::gregorian::date> m_school_holidays;
};

using CalendarRef = std::shared_ptr<const Calendar>;

} // end_of_namespace

#endif // end of include guard
