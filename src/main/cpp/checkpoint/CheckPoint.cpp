/*
 * CheckPoint.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: cedric
 */

#include "CheckPoint.h"
#include "hdf5.h"

#include <core/ClusterType.h>
#include <core/Health.h>
#include <pop/Person.h>
#include <util/Errors.h>
#include <util/InstallDirs.h>

#include <cstring>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

using namespace std;

namespace stride {
namespace checkpoint {

CheckPoint::CheckPoint(std::string filename, bool create_mode)
{
	if (create_mode) {
		CreateFile(filename);
	} else {
		OpenFile(filename);
	}
}

void CheckPoint::CreateFile(std::string filename)
{
	m_file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	// TODO: add a lot of preknown data such as Holidays, info about disease,...
}

void CheckPoint::WriteConfig(const SingleSimulationConfig& conf)
{
	std::shared_ptr<CommonSimulationConfig> common_config = conf.common_config;
	std::shared_ptr<LogConfig> log_config = conf.log_config;

	hid_t group = H5Gcreate2(m_file, "Config", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Gclose(group);

	group = H5Gcreate2(m_file, "Population", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Gclose(group);
	WriteFileDSet(common_config->disease_config_file_name, "disease");
	WriteFileDSet(common_config->contact_matrix_file_name, "contact");
	WriteFileDSet(conf.GetPopulationPath(), "popconfig");
	if (conf.GetGeodistributionProfilePath().size() != 0) {
		WriteFileDSet(conf.GetGeodistributionProfilePath(), "geoconfig");
	}
	if (conf.GetReferenceHouseholdsPath().size() != 0) {
		WriteFileDSet(conf.GetReferenceHouseholdsPath(), "contact");
	}

	group = H5Gopen2(m_file, "Config", H5P_DEFAULT);
	hsize_t dims = 2;
	hid_t dataspace = H5Screate_simple(1, &dims, NULL);
	bool bools[2];
	bools[0] = common_config->track_index_case;
	bools[1] = log_config->generate_person_file;
	hid_t attr = H5Acreate2(group, "bools", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr, H5T_NATIVE_INT, bools);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 4;
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "uints", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	unsigned int uints[4];
	uints[0] = common_config->rng_seed;
	uints[1] = common_config->number_of_days;
	uints[2] = common_config->number_of_survey_participants;
	uints[3] = (unsigned int)log_config->log_level;
	H5Awrite(attr, H5T_NATIVE_UINT, uints);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = 3;
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "doubles", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	double doubles[3];
	doubles[0] = common_config->r0;
	doubles[1] = common_config->seeding_rate;
	doubles[2] = common_config->immunity_rate;
	H5Awrite(attr, H5T_NATIVE_DOUBLE, doubles);
	H5Sclose(dataspace);
	H5Aclose(attr);

	dims = log_config->output_prefix.size();
	dataspace = H5Screate_simple(1, &dims, NULL);
	attr = H5Acreate2(group, "prefix", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT);
	char prefix[log_config->output_prefix.size()];
	strncpy(prefix, log_config->output_prefix.c_str(), sizeof(prefix));
	prefix[sizeof(prefix) - 1] = 0;
	H5Awrite(attr, H5T_NATIVE_CHAR, prefix);
	H5Sclose(dataspace);
	H5Aclose(attr);

	H5Gclose(group);
}

void CheckPoint::WriteConfig(const MultiSimulationConfig& conf)
{
	hid_t f = m_file;
	auto singles = conf.GetSingleConfigs();
	if (singles.size() == 1) {
		WriteConfig(singles[0]);
		return;
	}
	for (unsigned int i = 0; i < singles.size(); i++) {
		std::stringstream ss;
		ss << "Simulation " << i;

		m_file = H5Gcreate2(m_file, ss.str().c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		WriteConfig(singles[i]);
		H5Gclose(m_file);
	}
	m_file = f;
}

void CheckPoint::OpenFile(std::string filename) { m_file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); }

void CheckPoint::WriteDate(const Calendar& cal)
{
	std::size_t day = cal.GetDay();
	std::size_t month = cal.GetMonth();
	std::size_t year = cal.GetYear();

	std::stringstream ss;
	ss << year;
	if (month < 10) {
		ss << 0;
	}
	ss << month;
	if (day < 10) {
		ss << 0;
	}
	ss << day;
	unsigned int date = std::stoi(ss.str());
	// TODO: write as attribute to dataset
}

void CheckPoint::WriteHolidays(const boost::property_tree::ptree& holidays_ptree)
{
	std::vector<boost::gregorian::date> holidays;
	std::vector<boost::gregorian::date> school_holidays;
	// Read in holidays.
	for (int i = 1; i < 13; i++) {
		const std::string month = std::to_string(i);
		const std::string year = holidays_ptree.get<std::string>("year", "2016");

		// Read in general holidays.
		const std::string general_key = "general." + month;
		for (auto& date : holidays_ptree.get_child(general_key)) {
			const std::string date_string = year + "-" + month + "-" + date.second.get_value<std::string>();
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			holidays.push_back(new_holiday);
		}

		// Read in school holidays.
		const std::string school_key = "school." + month;
		for (auto& date : holidays_ptree.get_child(school_key)) {
			const std::string date_string = year + "-" + month + "-" + date.second.get_value<std::string>();
			const auto new_holiday = boost::gregorian::from_simple_string(date_string);
			school_holidays.push_back(new_holiday);
		}
	}
	unsigned int school_dset[school_holidays.size()];
	unsigned int holi_dset[holidays.size()];

	for (unsigned int i = 0; i < holidays.size(); i++) {
		holi_dset[i] = std::stoi(to_iso_string(holidays[i]));
	}

	for (unsigned int i = 0; i < school_holidays.size(); i++) {
		school_dset[i] = std::stoi(to_iso_string(school_holidays[i]));
	}

	// write the general holidays
	hsize_t dims[1] = {holidays.size() - 1};
	hid_t dataspace = H5Screate_simple(1, dims, NULL);
	hid_t dataset = H5Dcreate2(m_file, "holidays", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dataset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, holi_dset);
	H5Sclose(dataspace);
	H5Dclose(dataset);

	// write the school holidays
	dims[1] = school_holidays.size() - 1;
	dataspace = H5Screate_simple(1, dims, NULL);
	dataset =
	    H5Dcreate2(m_file, "school_holidays", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dataset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, school_dset);
	H5Sclose(dataspace);
	H5Dclose(dataset);
}

void CheckPoint::WritePopulation(const Population& pop)
{
	unsigned int dset[pop.size() - 1][5 + NumOfClusterTypes() - 1];
	unsigned int i = 0;
	for (const auto& p : pop) {

		// Basic Data
		dset[i][0] = p.GetId();
		dset[i][1] = p.GetAge();
		dset[i][2] = p.GetGender();
		dset[i][3] = p.IsParticipatingInSurvey();

		// Health data
		dset[i][4] = (unsigned int)p.GetHealth().GetHealthStatus();

		// Cluster data
		for (unsigned int j = 0; j < NumOfClusterTypes(); j++) {
			ClusterType temp = (ClusterType)j;
			if (ToString(temp) == "Null") {
				continue;
			}
			dset[i][5 + j] = p.GetClusterId(temp);
		}
		i++;
	}
	hsize_t dims[2] = {pop.size() - 1, 5 + NumOfClusterTypes() - 1};
	hid_t dataspace = H5Screate_simple(2, dims, NULL);
	// TODO : Get name to  be the date
	hid_t dataset =
	    H5Dcreate2(m_file, "Population/", H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset);
	H5Dclose(dataset);
	H5Sclose(dataspace);
}

void CheckPoint::WriteFileDSet(const std::string& filename, const std::string& setname)
{
	boost::filesystem::path path = util::InstallDirs::GetDataDir();
	boost::filesystem::path filep(filename);
	boost::filesystem::path fullpath = path / filep;
	if (!is_regular_file(fullpath)) {
		FATAL_ERROR("Unable to find file: " + fullpath.string());
	}
	std::ifstream f(fullpath.string());

	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	std::vector<char> dataset;
	std::copy(str.begin(), str.end(), std::back_inserter(dataset));

	hid_t group = H5Gopen2(m_file, "Config", H5P_DEFAULT);
	hsize_t dims[1] = {dataset.size() - 1};
	hid_t dataspace = H5Screate_simple(1, dims, NULL);
	hid_t dset =
	    H5Dcreate2(group, setname.c_str(), H5T_STD_I32BE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(*dataset.begin()));
	H5Dclose(dset);
	H5Sclose(dataspace);
	H5Gclose(group);
}

} /* namespace checkpoint */
} /* namespace stride */
