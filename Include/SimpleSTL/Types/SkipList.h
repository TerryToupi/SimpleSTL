#pragma once

#include <cassert>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>


template<class NodePtr, class ValueRef, class ValuePtr>
class SkipListIterator
{
public:
	using iterator_category = std::forward_iterator_tag;
	using vaule_type = std::remove_reference_t<ValueRef>;
	using difference_type = std::ptrdiff_t;
	using pointer = ValuePtr;
	using referance = ValueRef;

	SkipListIterator() noexcept = default;
	explicit SkipListIterator(NodePtr n) noexcept 
		:	m_node(n) { }

	referance operator*() const noexcept { return m_node->kv; }
	pointer operator->() const noexcept { return std::addressof(m_node->kv); }

	SkipListIterator& operator++() noexcept
	{
		m_node = m_node->next[0];
		return *this;
	}
	SkipListIterator operator++(int) noexcept
	{
		SkipListIterator tmp(*this);
		++(*this);
		return tmp;
	}

	friend bool operator==(const SkipListIterator& a, const SkipListIterator& b) noexcept
	{
		return a.m_node == b.m_node;
	}
	friend bool operator!=(const SkipListIterator& a, const SkipListIterator& b) noexcept
	{
		return !(a == b);
	}

	NodePtr node() const noexcept { return m_node; }

private:
	NodePtr m_node = nullptr;
};


template<
	class Key,
	class Value,
	class Compare = std::less<Key>,
	class Alloc = std::allocator<std::pair<const Key, Value>>,
	int MaxLevel = 16,
	int PNumerator = 1,
	int PDenominator = 2
>
class SkipList
{
private:
	static_assert(MaxLevel >= 2, "Max level must be more or equal than 2");
	static_assert(PNumerator > 0 && PDenominator > 0 && PNumerator < PDenominator, "P must be 0 < P < 1");

public:
	using key_type = Key;
	using mapped_type = Value;
	using value_type = std::pair<const Key, Value>;
	using size_type = size_t;
	using diff_type = std::ptrdiff_t;
	using key_compare = Compare;
	using allocator_type = Alloc;

private:
	using byte_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;
	using byte_traits = std::allocator_traits<byte_alloc>;

	struct Node
	{
		value_type kv;
		uint8_t height = 1;
		Node* next[1];

		Node(const value_type& v, uint8_t h)
			: kv(v), height(h) { }

		Node(value_type&& v, uint8_t h)
			: kv(std::move(v)), height(h) { }
	};

	static constexpr size_t node_bytes(uint8_t height) noexcept
	{
		return sizeof(Node) + (static_cast<size_t>(height) - 1) * sizeof(Node*);
	}

	template<class T>
	Node* create_node(T&& t, uint8_t height)
	{
		assert(height >= 1 && height <= MaxLevel);

		const size_t bytes = node_bytes(height);
		std::byte* memory = byte_traits::allocate(m_byte_alloc, bytes);

		Node* n = nullptr;
		try
		{
			n = ::new (static_cast<void*>(memory)) Node(std::forward<T>(t), height);
			for (size_t i = 0; i < height; ++i)
			{
				n->next[i] = nullptr;
			}
		}
		catch (...)
		{
			byte_traits::deallocate(m_byte_alloc, memory, bytes);
			throw;
		}

		return n;
	}

	void destroy_node(Node* n) noexcept
	{
		if (!n)
			return;
		
		const size_t bytes = node_bytes(n->height);
		auto* memory = reinterpret_cast<std::byte*>(n);

		n->~Node();
		byte_traits::deallocate(m_byte_alloc, memory, bytes);
	}

	static constexpr bool key_less(const Compare& comp, const Key& a, const Key& b)
	{
		return comp(a, b);
	}

	static constexpr bool key_eq(const Compare& comp, const Key& a, const Key& b)
	{
		return !comp(a, b) && !comp(b, a);
	}

public:
	using iterator = SkipListIterator<Node*, value_type&, value_type*>;
	using const_iterator = SkipListIterator<const Node*, const value_type&, const value_type*>;

	SkipList()
		: SkipList(Compare{}, Alloc{}) { }

	explicit SkipList(const Compare& comp, const Alloc& alloc = Alloc{})
		: m_comp(comp), m_alloc(alloc), m_byte_alloc(alloc), m_rng(std::random_device{}()) 
	{
		ini_head();
	}

	SkipList(const SkipList& other)
		: m_comp(other.m_comp),
		m_alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.m_alloc)),
		m_byte_alloc(m_alloc),
		m_rng(std::random_device{}()) 
	{
		ini_head();
		for (const auto& kv : other)
			insert(kv);
	}

	SkipList& operator=(const SkipList& other) 
	{
		if (this == &other) 
			return *this;

		clear();
		if constexpr (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) 
		{
			m_alloc = other.m_alloc;
			m_byte_alloc = byte_alloc(m_byte_alloc);
		}
		m_comp = other.m_comp;

		for (const auto& kv : other) 
			insert(kv);

		return *this;
	}

	SkipList(SkipList&& other) noexcept
		: m_comp(std::move(other.m_comp)),
		m_alloc(std::move(other.m_alloc)),
		m_byte_alloc(std::move(other.m_byte_alloc)),
		m_head(other.m_head),
		m_level(other.m_level),
		m_size(other.m_size),
		m_rng(std::move(other.m_rng)) 
	{
		other.m_head = nullptr;
		other.m_level = 1;
		other.m_size = 0;
	}

	SkipList& operator=(SkipList&& other) noexcept 
	{
		if (this == &other) 
			return *this;

		clear();
		destroy_head();

		if constexpr (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value) 
		{
			m_alloc = std::move(other.m_alloc);
			m_byte_alloc = std::move(other.m_byte_alloc);
		}

		m_comp = std::move(other.m_comp);
		m_head = other.m_head;
		m_level = other.m_level;
		m_size = other.m_size;
		m_rng = std::move(other.m_rng);

		other.m_head = nullptr;
		other.m_level = 1;
		other.m_size = 0;
		return *this;
	}

	~SkipList() 
	{
		clear();
		destroy_head();
	}

	allocator_type get_allocator() const noexcept { return m_alloc; }
	key_compare key_comp() const { return m_comp; }

	bool empty() const noexcept { return m_size == 0; }
	size_type size() const noexcept { return m_size; }

	iterator begin() noexcept { return iterator(m_head->next[0]); }
	iterator end() noexcept { return iterator(nullptr); }
	const_iterator begin() const noexcept { return const_iterator(m_head->next[0]); }
	const_iterator end() const noexcept { return const_iterator(nullptr); }
	const_iterator cbegin() const noexcept { return const_iterator(m_head->next[0]); }
	const_iterator cend() const noexcept { return const_iterator(nullptr); }

	void clear() noexcept 
	{
		if (!m_head) 
			return;

		Node* cur = m_head->next[0];
		while (cur) 
		{
			Node* nxt = cur->next[0];
			destroy_node(cur);
			cur = nxt;
		}

		for (std::size_t i = 0; i < MaxLevel; ++i) 
			m_head->next[i] = nullptr;

		m_level = 1;
		m_size = 0;
	}

	std::pair<iterator, bool> insert(const value_type& v) { return emplace_impl(v); }
	std::pair<iterator, bool> insert(value_type&& v) { return emplace_impl(std::move(v)); }

	std::pair<iterator, bool> insert_or_assign(const Key& key, Value value) 
	{
		auto it = find(key);
		if (it != end()) 
		{
			it->second = std::move(value);
			return { it, false };
		}

		return insert(value_type{ key, std::move(value) });
	}

	std::pair<iterator, bool> erase(const key_type& v) { return erase_impl(v); }

	iterator erase(iterator pos)
	{
		if (pos == end())
			return end();

		auto [it, erased] = erase_impl(pos->first);
		return it;
	}

	iterator find(const Key& key) noexcept 
	{
		Node* x = find_ge(key);
		if (x && key_eq(m_comp, x->kv.first, key)) 
			return iterator(x);

		return end();
	}
	const_iterator find(const Key& key) const noexcept 
	{
		const Node* x = find_ge_const(key);
		if (x && key_eq(m_comp, x->kv.first, key)) 
			return const_iterator(x);

		return end();
	}

	bool contains(const Key& key) const noexcept 
	{
		return find(key) != end();
	}

	iterator lower_bound(const Key& key) noexcept 
	{
		return iterator(find_ge(key));
	}
	const_iterator lower_bound(const Key& key) const noexcept 
	{
		return const_iterator(find_ge_const(key));
	}

private:
	void ini_head() 
	{
		value_type dummy{ Key{}, Value{} };
		m_head = create_node(std::move(dummy), (uint8_t)(MaxLevel));
		m_level = 1;
		m_size = 0;
	}

	void destroy_head() noexcept 
	{
		if (!m_head) 
			return;

		destroy_node(m_head);
		m_head = nullptr;
	}

	Node* find_ge(const Key& key) noexcept 
	{
		Node* x = m_head;
		for (int i = (int)(m_level) - 1; i >= 0; --i)
		{
			while (x->next[i] && key_less(m_comp, x->next[i]->kv.first, key)) 
				x = x->next[i];
		}

		return x->next[0];
	}

	const Node* find_ge_const(const Key& key) const noexcept 
	{
		const Node* x = m_head;
		for (int i = (int)(m_level) - 1; i >= 0; --i) 
		{
			while (x->next[i] && key_less(m_comp, x->next[i]->kv.first, key)) 
				x = x->next[i];
		}

		return x->next[0];
	}

	uint8_t random_height() 
	{
		uint8_t h = 1;
		while (h < MaxLevel) 
		{
			uint32_t r = m_dist(m_rng);
			if ((r % PDenominator) >= PNumerator) 
				break;

			++h;
		}
		return h;
	}

	template <class V>
	std::pair<iterator, bool> emplace_impl(V&& v) 
	{
		std::array<Node*, MaxLevel> update{};
		Node* x = m_head;

		for (int i = (int)(m_level) - 1; i >= 0; --i) 
		{
			while (x->next[i] && key_less(m_comp, x->next[i]->kv.first, v.first)) 
				x = x->next[i];

			update[i] = x;
		}

		x = x->next[0];
		if (x && key_eq(m_comp, x->kv.first, v.first))
			return { iterator(x), false };

		uint8_t h = random_height();
		if (h > m_level) 
		{
			for (size_t i = m_level; i < h; ++i) 
				update[i] = m_head;

			m_level = h;
		}

		Node* n = create_node(std::forward<V>(v), h);

		for (size_t i = 0; i < h; ++i) 
		{
			n->next[i] = update[i]->next[i];
			update[i]->next[i] = n;
		}

		++m_size;
		return { iterator(n), true };
	}

	template <class V>
	std::pair<iterator, bool> erase_impl(V&& v)
	{
		std::array<Node*, MaxLevel> update{};
		Node* x = m_head;

		for (int i = (int)(m_level) - 1; i >= 0; --i)
		{
			while (x->next[i] && key_less(m_comp, x->next[i]->kv.first, v))
				x = x->next[i];

			update[i] = x;
		}

		x = x->next[0];
		if (!x || !key_eq(m_comp, x->kv.first, v))
			return { iterator(x), false };

		for (size_t i = 0; i < x->height; ++i)
			update[i]->next[i] = x->next[i];

		Node* next = x->next[0];
		destroy_node(x);
		--m_size;

		while (m_level > 1 && m_head->next[m_level - 1] == nullptr)
			--m_level;

		return { iterator(next), true };
	}

private:
	using Random = std::mt19937;
	using Uint32Dist = std::uniform_int_distribution<uint32_t>;
	static constexpr uint32_t Uint32Limit = std::numeric_limits<uint32_t>::max();

	Compare		m_comp{};
	Alloc		m_alloc{};
	byte_alloc	m_byte_alloc{};
	Node*		m_head = nullptr;
	uint8_t		m_level = 1;
	size_t		m_size = 0;
	Uint32Dist	m_dist{ 0, Uint32Limit };
	Random		m_rng;
};
