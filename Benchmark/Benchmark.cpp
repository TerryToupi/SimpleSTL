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

void benchmark4()
{
	Benchmark::Keys keys{};

	auto optimized_less_2 = [](const std::string& a, const std::string& b)
		{
			size_t n = std::min<size_t>(2, std::min(a.size(), b.size()));

			int r = std::char_traits<char>::compare(a.data(), b.data(), n);
			if (r != 0)
				return r < 0;

			return a.size() < b.size();
		};
	auto optimized_less_1 = [](const std::string& a, const std::string& b)
		{
			size_t n = std::min<size_t>(1, std::min(a.size(), b.size()));

			int r = std::char_traits<char>::compare(a.data(), b.data(), n);
			if (r != 0)
				return r < 0;

			return a.size() < b.size();
		};

	SkipList<std::string, std::string>								mem;
	SkipList<std::string, std::string, decltype(optimized_less_1)>	mem1;
	SkipList<std::string, std::string, decltype(optimized_less_2)>	mem2;

	for (const auto& [k, v] : keys.GetKeys())
	{
		mem.insert({ k, v });
		mem1.insert({ k, v });
		mem2.insert({ k, v });
	}

	size_t dev1 = 0;
	size_t dev2 = 0;
	size_t total = 0;

	auto it1 = mem1.begin();
	auto it2 = mem2.begin();

	for (const auto& [base_key, _] : mem)
	{
		if (it1 == mem1.end())
			break;

		if (it2 == mem2.end())
			break;

		if (base_key != it1->first)
			++dev1;

		if (base_key != it2->first)
			++dev2;

		++it1;
		++it2;
		++total;
	}

	const double dev1_pct = 100.0 * (dev1 / total);
	const double dev2_pct = 100.0 * (dev2 / total);

	constexpr int COL_NAME = 30;
	constexpr int COL_DEV = 18;

	std::cout.imbue(std::locale(""));
	std::cout << std::fixed << std::setprecision(2);

	std::cout << "\n=== Comparator Ordering Deviation Benchmark ===\n";

	std::cout << std::left << std::setw(COL_NAME) << "Comparator"
		<< std::right << std::setw(COL_DEV) << "Deviation (%)\n";

	std::cout << std::string(COL_NAME + COL_DEV, '-') << "\n";

	std::cout << std::left << std::setw(COL_NAME) << "Prefix 1 character"
		<< std::right << std::setw(COL_DEV) << dev1_pct << "\n";

	std::cout << std::left << std::setw(COL_NAME) << "Prefix 2 characters"
		<< std::right << std::setw(COL_DEV) << dev2_pct << "\n";
}

void benchmark3()
{
	Benchmark::Keys keys{};

	// optimized less funciton
	static constexpr uint8_t CHARACTERS_TO_COMPARE = 2;
	auto optimized_less = [](const std::string& a, const std::string& b)
		{
			size_t n = std::min<size_t>(CHARACTERS_TO_COMPARE, std::min(a.size(), b.size()));

			int r = std::char_traits<char>::compare(a.data(), b.data(), n);
			if (r != 0)
				return r < 0;

			return a.size() < b.size();
		};

	SkipList<std::string, std::string>								mem;
	SkipList<std::string, std::string, decltype(optimized_less)>	mem1;

	auto t0 = timestamp();
	for (const auto [key, value] : keys.GetKeys())
		mem.insert({ key, value });
	auto t1 = timestamp();

	auto t3 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
	{
		const auto it = mem.find(keys.PickRandomKey());
		assert(!it->second.empty());
	}
	auto t4 = timestamp();

	auto t5 = timestamp();
	for (const auto [key, value] : keys.GetKeys())
		mem1.insert({ key, value });
	auto t6 = timestamp();

	auto t7 = timestamp();
	for (int i = 0; i < keys.GetNumOfKeys(); ++i)
	{
		const auto it = mem.find(keys.PickRandomKey());
		assert(!it->second.empty());
	}
	auto t8 = timestamp();

	constexpr int COL_NAME = 28;
	constexpr int COL_TIME = 18;
	constexpr int COL_OPS = 18;

	constexpr double NS_PER_SEC = 1e9;

	const auto def_insert_ns = t1 - t0;
	const auto def_find_ns = t4 - t3;
	const auto opt_insert_ns = t6 - t5;
	const auto opt_find_ns = t8 - t7;

	const double def_insert_ops =
		keys.GetNumOfKeys() / (def_insert_ns / NS_PER_SEC);
	const double def_find_ops =
		keys.GetNumOfKeys() / (def_find_ns / NS_PER_SEC);
	const double opt_insert_ops =
		keys.GetNumOfKeys() / (opt_insert_ns / NS_PER_SEC);
	const double opt_find_ops =
		keys.GetNumOfKeys() / (opt_find_ns / NS_PER_SEC);

	std::cout.imbue(std::locale(""));
	std::cout << std::fixed << std::setprecision(3);

	std::cout << "\n=== SkipList Comparator Benchmark ===\n";

	std::cout << std::left << std::setw(COL_NAME) << "Operation"
		<< std::right << std::setw(COL_TIME) << "Time (ms)"
		<< std::right << std::setw(COL_OPS) << "Ops/sec\n";

	std::cout << std::string(COL_NAME + COL_TIME + COL_OPS, '-') << "\n";

	auto print_row = [&](const char* name, double time_ns, double ops)
		{
			std::cout << std::left << std::setw(COL_NAME) << name
				<< std::right << std::setw(COL_TIME) << (time_ns / 1e6)
				<< std::right << std::setw(COL_OPS) << ops
				<< "\n";
		};

	print_row("Default Insert", def_insert_ns, def_insert_ops);
	print_row("Default Find", def_find_ns, def_find_ops);
	print_row("Optimized Insert", opt_insert_ns, opt_insert_ops);
	print_row("Optimized Find", opt_find_ns, opt_find_ops);
}

void benchmark2()
{
	Benchmark::Keys keys{};

	SkipList<std::string, std::string>				mem;
	std::map<std::string, std::string>				mem2;
	std::unordered_map<std::string, std::string>	mem3;
	
	std::random_device rd{};
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(0, 2);

	const auto t1 = timestamp();
	for (int i{}; i < keys.GetNumOfKeys(); ++i)
	{
		int choice = dist(gen);

		switch (choice)
		{
		case 0:
			mem.insert(keys.PickRandomKV());
			break;
		case 1:
			mem.find(keys.PickRandomKey());
			break;
		case 2:
			mem.erase(keys.PickRandomKey());
			break;
		default:
			break;
		}
	}
	const auto t2 = timestamp();

	const auto t3 = timestamp();
	for (int i{}; i < keys.GetNumOfKeys(); ++i)
	{
		int choice = dist(gen);

		switch (choice)
		{
		case 0:
			mem2.insert(keys.PickRandomKV());
			break;
		case 1:
			mem2.find(keys.PickRandomKey());
			break;
		case 2:
			mem2.erase(keys.PickRandomKey());
			break;
		default:
			break;
		}
	}
	const auto t4 = timestamp();

	const auto t5 = timestamp();
	for (int i{}; i < keys.GetNumOfKeys(); ++i)
	{
		int choice = dist(gen);

		switch (choice)
		{
		case 0:
			mem2.insert(keys.PickRandomKV());
			break;
		case 1:
			mem2.find(keys.PickRandomKey());
			break;
		case 2:
			mem2.erase(keys.PickRandomKey());
			break;
		default:
			break;
		}
	}
	const auto t6 = timestamp();

	constexpr int COL_NAME = 18;
	constexpr int COL_TIME = 18;
	constexpr int COL_OPS = 18;

	constexpr double NS_PER_SEC = 1e9;

	const auto skip_ns = t2 - t1;
	const auto map_ns = t4 - t3;
	const auto hash_ns = t6 - t5;

	const double skip_ops = keys.GetNumOfKeys() / (skip_ns / NS_PER_SEC);
	const double map_ops = keys.GetNumOfKeys() / (map_ns / NS_PER_SEC);
	const double hash_ops = keys.GetNumOfKeys() / (hash_ns / NS_PER_SEC);

	std::cout.imbue(std::locale(""));
	std::cout << std::fixed << std::setprecision(3);

	std::cout << "\n=== Workload Benchmark ===\n";

	std::cout << std::left << std::setw(COL_NAME) << "Structure"
		<< std::right << std::setw(COL_TIME) << "Time (ms)"
		<< std::right << std::setw(COL_OPS) << "Ops/sec\n";

	std::cout << std::string(COL_NAME + COL_TIME + COL_OPS, '-') << "\n";

	auto print_row = [&](const char* name, double time_ns, double ops)
		{
			std::cout << std::left << std::setw(COL_NAME) << name
				<< std::right << std::setw(COL_TIME) << (time_ns / 1e6)
				<< std::right << std::setw(COL_OPS) << ops
				<< "\n";
		};

	print_row("SkipList", skip_ns, skip_ops);
	print_row("Map", map_ns, map_ops);
	print_row("HashMap", hash_ns, hash_ops);
}

void benchmark1()
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

	std::cout << "\n=== Throughput Benchmark ===\n";

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

int main() 
{
	benchmark1();
	benchmark2();
	benchmark3();
	benchmark4();

	return 1;
}