#include <SimpleSTL/Types/SkipList.h>
#include <unordered_map>
#include <map>

#include <Keys.h>

#include <iostream>
#include <string>
#include <random>
#include <cstddef>
#include <chrono>

inline uint64_t timestamp()
{
	using clock = std::chrono::steady_clock;
	return std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now().time_since_epoch()).count();
}

int main() 
{
	Benchmark::Keys keys{};
	SkipList<std::string, std::string> mem;
	std::unordered_map<std::string, std::string> mem2;
	std::map<std::string, std::string> mem3;

	for (const auto [key, value] : keys.GetKeys())
		mem.insert({ key, value });

	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
		mem.erase(keys.PickRandomKey());

	mem.clear();

	//auto t3 = timestamp();
	//for (int i = 0; i < INSERTS; ++i)
	//	mem2.insert({ std::move(random_word(5, 12)), std::move(random_word(20, 120)) });
	//auto t4 = timestamp();

	//mem2.clear();

	//auto t5 = timestamp();
	//for (int i = 0; i < INSERTS; ++i)
	//	mem3.insert({ std::move(random_word(5, 12)), std::move(random_word(20, 120)) });
	//auto t6 = timestamp();

	//mem3.clear();

	//auto skip_ns = t2 - t1;
	//auto hash_ns = t4 - t3;
	//auto map_ns =  t6 - t5;

	//constexpr double NS_PER_SEC = 1e9;
	//double skip_ops = INSERTS / (skip_ns / NS_PER_SEC);
	//double hash_ops = INSERTS / (hash_ns / NS_PER_SEC);
	//double map_ops = INSERTS / (map_ns / NS_PER_SEC);

	//std::cout << "\n=== Insert Benchmark ===\n";
	//std::cout << std::left << std::setw(12) << "Structure"
	//	<< std::right << std::setw(15) << "Time (ms)"
	//	<< std::right << std::setw(18) << "Ops/sec\n";
	//std::cout << std::string(45, '-') << "\n";

	//std::cout << std::left << std::setw(12) << "SkipList"
	//	<< std::right << std::setw(15) << skip_ns / 1e6
	//	<< std::right << std::setw(18) << static_cast<uint64_t>(skip_ops) << "\n";

	//std::cout << std::left << std::setw(12) << "Map"
	//	<< std::right << std::setw(15) << map_ns / 1e6
	//	<< std::right << std::setw(18) << static_cast<uint64_t>(map_ops) << "\n";

	//std::cout << std::left << std::setw(12) << "HashMap"
	//	<< std::right << std::setw(15) << hash_ns / 1e6
	//	<< std::right << std::setw(18) << static_cast<uint64_t>(hash_ops) << "\n";

}