#include <SimpleSTL/Types/SkipList.h>
#include <unordered_map>
#include <map>

#include <Keys.h>

#include <iostream>
#include <string>
#include <random>
#include <cstddef>
#include <chrono>
#include <cassert>

inline uint64_t timestamp()
{
	using clock = std::chrono::steady_clock;
	return std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now().time_since_epoch()).count();
}

int main() 
{
	Benchmark::Keys keys{};

	SkipList<std::string, std::string>				mem;
	std::map<std::string, std::string>				mem2;
	std::unordered_map<std::string, std::string>	mem3;

	// Skiplist section
	auto t0 = timestamp();
	for (const auto [key, value] : keys.GetKeys())
		mem.insert({ key, value });
	auto t1 = timestamp();

	auto t2 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
	{
		const auto it = mem.find(keys.PickRandomKey());
		assert(!it->second.empty());
	}
	auto t3 = timestamp();

	auto t4 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
		mem.erase(keys.PickRandomKey());
	auto t5 = timestamp();

	mem.clear();

	// map section
	auto t6 = timestamp();
	for (const auto [key, value] : keys.GetKeys())
		mem2.insert({ key, value });
	auto t7 = timestamp();

	auto t8 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
	{
		const auto it = mem2.find(keys.PickRandomKey());
		assert(!it->second.empty());
	}
	auto t9 = timestamp();

	auto t10 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
		mem2.erase(keys.PickRandomKey());
	auto t11 = timestamp();

	mem2.clear();

	// Hashmap
	auto t12 = timestamp();
	for (const auto [key, value] : keys.GetKeys())
		mem3.insert({ key, value });
	auto t13 = timestamp();

	auto t14 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
	{
		const auto it = mem3.find(keys.PickRandomKey());
		assert(!it->second.empty());
	}
	auto t15 = timestamp();

	auto t16 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
		mem3.erase(keys.PickRandomKey());
	auto t17 = timestamp();

	mem3.clear();


	constexpr double NS_PER_SEC = 1e9;

	//skiplist numbers
	auto skip_insert_ns = t1 - t0;
	auto skip_find_ns = t3 - t2;
	auto skip_erase_ns = t5 - t4;

	double skip_insert_ops = keys.GetNumOfKeys() / (skip_insert_ns / NS_PER_SEC);
	double skip_find_ops = keys.GetNumOfKeys() / (skip_find_ns / NS_PER_SEC);
	double skip_erase_ops = keys.GetNumOfKeys() / (skip_erase_ns / NS_PER_SEC);

	//map numbers
	auto map_insert_ns = t7 - t6;
	auto map_find_ns = t9 - t8;
	auto map_erase_ns = t11 - t10;

	double map_insert_ops = keys.GetNumOfKeys() / (map_insert_ns / NS_PER_SEC);
	double map_find_ops = keys.GetNumOfKeys() / (map_find_ns / NS_PER_SEC);
	double map_erase_ops = keys.GetNumOfKeys() / (map_erase_ns / NS_PER_SEC);

	//hash numbers
	auto hash_insert_ns = t13 - t12;
	auto hash_find_ns = t15 - t14;
	auto hash_erase_ns = t17 - t16;

	double hash_insert_ops = keys.GetNumOfKeys() / (hash_insert_ns / NS_PER_SEC);
	double hash_find_ops = keys.GetNumOfKeys() / (hash_find_ns / NS_PER_SEC);
	double hash_erase_ops = keys.GetNumOfKeys() / (hash_erase_ns / NS_PER_SEC);

	constexpr int COL_NAME = 18;
	constexpr int COL_TIME = 18;
	constexpr int COL_OPS = 18;

	std::cout.imbue(std::locale(""));
	std::cout << std::fixed << std::setprecision(3);

	std::cout << "\n=== Insert Benchmark ===\n";

	std::cout << std::left << std::setw(COL_NAME) << "Structure"
		<< std::right << std::setw(COL_TIME) << "Time (ms)"
		<< std::right << std::setw(COL_OPS) << "Ops/sec\n";

	std::cout << std::string(COL_NAME + COL_TIME + COL_OPS, '-') << "\n";

	auto print_row = [&](const char* name, double time_ns, uint64_t ops)
		{
			std::cout << std::left << std::setw(COL_NAME) << name
				<< std::right << std::setw(COL_TIME) << (time_ns / 1e6)
				<< std::right << std::setw(COL_OPS) << ops
				<< "\n";
		};

	print_row("SkipList Insert", skip_insert_ns, skip_insert_ops);
	print_row("SkipList Get", skip_find_ns, skip_find_ops);
	print_row("SkipList Erase", skip_erase_ns, skip_erase_ops);

	print_row("Map Insert", map_insert_ns, map_insert_ops);
	print_row("Map Get", map_find_ns, map_find_ops);
	print_row("Map Erase", map_erase_ns, map_erase_ops);

	print_row("Hash Insert", hash_insert_ns, hash_insert_ops);
	print_row("Hash Get", hash_find_ns, hash_find_ops);
	print_row("Hash Erase", hash_erase_ns, hash_erase_ops);

}