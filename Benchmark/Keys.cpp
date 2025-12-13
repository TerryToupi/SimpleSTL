#include <Keys.h>
#include <random>
#include <cassert>

static inline std::string random_word(std::size_t min_len = 3, std::size_t max_len = 12) 
{
	static thread_local std::mt19937 rng{ std::random_device{}() };

	std::uniform_int_distribution<std::size_t> len_dist(min_len, max_len);
	std::uniform_int_distribution<int> char_dist('a', 'z');

	const std::size_t len = len_dist(rng);
	std::string s;
	s.reserve(len);

	for (std::size_t i = 0; i < len; ++i)
		s.push_back(static_cast<char>(char_dist(rng)));

	return s;
}

Benchmark::Keys::Keys(uint32_t numOfPairs, uint32_t maxKeySize, uint32_t maxValueSize, size_t range_size, double zipf_theta, bool suffel_within_range)
	:	m_numOfPairs(numOfPairs), m_maxKeySize(maxKeySize), m_maxValueSize(maxValueSize), m_range_size(range_size), m_theta(zipf_theta), m_shuffle(suffel_within_range)
{
	assert(m_range_size > 0);
	assert(m_range_size <= m_numOfPairs);
	assert(m_theta > 0.0 && m_theta < 1.0);

	m_range_offset = 0;
	m_picker_enabled = true;

	if (m_shuffle)
		m_permutations.resize(m_range_size);

	m_storge.reserve(numOfPairs);
	for (uint32_t i{}; i < m_numOfPairs; ++i)
		m_storge.push_back({ std::move(random_word(maxKeySize / 2, maxKeySize)), std::move(random_word(maxValueSize / 2, maxValueSize)) });

	pick_new_range();
}

const Benchmark::Keys::KeyType& Benchmark::Keys::PickRandomKey()
{
	assert(m_picker_enabled);

	if (m_range_offset >= m_range_size)
		pick_new_range();

	size_t idx = m_shuffle ? m_permutations[m_range_offset] : m_range_offset;

	const auto& key = m_storge[m_range_start + idx].first;
	++m_range_offset;

	return key;
}

void Benchmark::Keys::pick_new_range()
{
	const size_t max_start = m_storge.size() - m_range_size;

	m_range_start = zipf_sample(max_start + 1);
	m_range_offset = 0;

	if (m_shuffle)
	{
		for (size_t i = 0; i < m_range_size; ++i)
			m_permutations[i] = i;

		std::shuffle(m_permutations.begin(), m_permutations.end(), m_rng);
	}
}

size_t Benchmark::Keys::zipf_sample(size_t max_start)
{
	double z = m_uni(m_rng);
	double alpha = 1.0 / (1.0 - m_theta);
	return static_cast<size_t>(max_start * std::pow(z, alpha));
}

