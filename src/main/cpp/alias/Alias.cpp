#include "Alias.h"

#include <exception>
#include <assert.h>
#include <math.h>
#include "AliasUtil.h"

namespace stride {
namespace alias {

void NormalizeProbabilities(std::vector<double>& probabilities)
{
	double sum = 0.0;
	for (auto item : probabilities) {
		sum += item;
	}

	for (auto& item : probabilities) {
		item /= sum;
	}
}

Alias Alias::CreateDistribution(std::vector<double> probabilities, util::Random& rng)
{
	assert(probabilities.size() > 0);
	if (probabilities.size() <= 0) {
		throw EmptyProbabilityException();
	}
	NormalizeProbabilities(probabilities);
	std::size_t n = probabilities.size();
	std::vector<double> prob(n);
	std::vector<std::size_t> alias(n);
	std::vector<std::size_t> small, large;
	for (auto& i : probabilities) {
		i *= n;
	}
	for (std::size_t i = 0; i < probabilities.size(); i++) {
		if (probabilities[i] < 1.0) {
			small.push_back(i);
		} else {
			large.push_back(i);
		}
	}

	while (!(small.empty() || large.empty())) {
		std::size_t l = large.front();
		large.erase(large.begin());
		std::size_t g = small.front();
		small.erase(small.begin());
		prob[l] = probabilities[l];
		alias[l] = g;

		probabilities[g] = probabilities[g] + probabilities[l] - 1;
		if (probabilities[g] >= 1) {
			large.push_back(g);
		} else {
			small.push_back(g);
		}
	}

	for (auto g : large) {
		prob[g] = 1;
	}

	for (auto l : small) {
		prob[l] = 1;
	}
	return Alias(std::move(alias), std::move(prob), rng);
}

std::size_t Alias::Next()
{
	std::size_t roll = (*m_random)(m_alias.size());
	double flip = m_random->NextDouble();
	if (flip <= m_prob[roll]) {
		return roll;
	} else {
		return m_alias[roll];
	}
}

} // end-of-namespace
} // end-of-namespace
