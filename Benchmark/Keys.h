#pragma once

#include <vector>
#include <string>
#include <random>

namespace Benchmark
{
	class Keys
	{
	public:
		using KeyType = std::string;
		using ValueType = std::string;
		using KVContainer = std::vector<std::pair<KeyType, ValueType>>;

		explicit Keys(
			uint32_t numOfPairs = 1'000'000,
			uint32_t maxKeySize = 20,
			uint32_t maxValueSize = 120,
			size_t range_size = 1024,
			double zipf_theta = 0.99,
			bool suffel_within_range = true
		);

		const KVContainer& GetKeys() const { return m_storge; }
		uint32_t GetNumOfKeys()		 const { return m_numOfPairs; };

		const KeyType& PickRandomKey();
		const std::pair<KeyType, ValueType>& PickRandomKV();

	private:
		void pick_new_range();
		size_t zipf_sample(size_t max_start);

	private:
		using Random = std::mt19937;
		using UniformRealDoubleDist = std::uniform_real_distribution<double>;

		uint32_t m_numOfPairs = 0;
		uint8_t m_maxKeySize = 0;
		uint8_t m_maxValueSize = 0;
		KVContainer m_storge{};

		bool m_picker_enabled = false;
		bool m_shuffle = false;

		size_t m_range_size = 0;
		size_t m_range_start = 0;
		size_t m_range_offset = 0;

		double m_theta = 0.99;
		
		Random m_rng{ std::random_device{}() };
		UniformRealDoubleDist m_uni{ 0.0, 1.0 };

		std::vector<size_t> m_permutations;
	};
}