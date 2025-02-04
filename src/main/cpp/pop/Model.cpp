#include <iostream>
#include "Model.h"

namespace stride {
namespace population {

ModelRef Model::Parse(const boost::property_tree::ptree& pt)
{
	auto root = pt.get_child("population_model");
	std::map<util::InclusiveRange<int>, double> town_distribution;
	for (const auto& p : root.get_child("town_distribution")) {
		town_distribution[util::InclusiveRange<int>::Parse(p.second)] = p.second.get<double>("probability");
	}

	return std::make_shared<Model>(
	    root.get<int>("school_size"), root.get<int>("school_cluster_size"), root.get<int>("college_size"),
	    root.get<int>("college_cluster_size"), root.get<int>("workplace_size"), root.get<int>("community_size"),
	    root.get<double>("search_radius"), root.get<int>("population_size"), root.get<double>("city_ratio"),
	    town_distribution, util::InclusiveRange<int>::Parse(root.get_child("school_age")),
	    util::InclusiveRange<int>::Parse(root.get_child("college_age")), root.get<double>("college_ratio"),
	    root.get<double>("college_commute_ratio"),
	    util::InclusiveRange<int>::Parse(root.get_child("employable_age")), root.get<double>("employed_ratio"),
	    root.get<double>("work_commute_ratio"));
}

} // namespace population
} // namespace stride
