/*
 * CheckPoint.h
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#ifndef CHECKPOINT_H_INCLUDED
#define CHECKPOINT_H_INCLUDED

#include <memory>
#include "calendar/Calendar.h"
#include "hdf5.h"
#include "pop/Population.h"

namespace stride {
namespace checkpoint {

class CheckPoint
{
public:
	/// Constructor The string is the file for the checkpoints. The boolean is false if the file needs to be
	/// created.
	CheckPoint(std::string, bool create_mode = false);

	/// Loads a checkpoint from the file in the constructor. The unsigned int tells which checkpoint to use.
	void LoadCheckPoint(unsigned int);

	/// Saves the current simulation to a checkpoint.
	void SaveCheckPoint();

private:
	/// Creates the wanted file
	void CreateFile(std::string);

	/// Opens the wanted file
	void OpenFile(std::string);

	/// Writes the holidays.
	void WriteHolidays(const boost::property_tree::ptree&);

	/// Loads the holidays into a boost::property_tree::ptree.
	std::unique_ptr<boost::property_tree::ptree> LoadHolidays();

	/// Writes Contact Matrices.
	void WriteMatrices();

	/// Loads Contact Matrices.

	/// Writes the disease.
	void WriteDisease(const boost::property_tree::ptree&);

	/// Loads the disease into a boost::property_tree::ptree.
	std::unique_ptr<boost::property_tree::ptree> LoadDisease();

	/// Writes the current date to the checkpoint.
	void WriteDate(const Calendar&);

	/// Loads the current date from a checkpoint.
	boost::gregorian::date LoadDate();

	/// Writes the current population to a checkpoint.
	void WritePopulation(const Population&);

	/// Loads the asked population from the given checkpoint.
	Population LoadPopulation(unsigned int);

	hid_t m_file;
};

} /* namespace checkpoint */
} /* namespace stride */

#endif /* CHECKPOINT_H_ */
