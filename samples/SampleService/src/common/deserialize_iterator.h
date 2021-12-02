/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#pragma once
#include "serialize_common.h"
#include "traits.h"
#include "memory_view.h"
#include <cassert>

class DeserializeIterator
{
	DeserializeIterator(const DeserializeIterator&) = delete;
	DeserializeIterator& operator=(const DeserializeIterator&) = delete;
public:
	// Use this constructor, only when size of buffer is unknown (like in FilterGetMessage)
	DeserializeIterator(const void* mem);

	DeserializeIterator(DeserializeIterator&&) = default;

	DeserializeIterator(const void* mem, size_t sz);

	template <typename T>
	T get()
	{
		return get_impl<T>();
	}

	// cannot use get specialization, kind of sad
	memory_view get_variable_buffer();

	bool finalize() const;
	bool has_error() const;

private:
	template <typename T>
	traits::enable_if_t<traits::is_unspecified_v<T>, T>
		get_impl()
	{
		static_assert(false, "Supported getters: integrals, enums, POD structs, wstring_t, array_t, UNICODE_STRING, memory_view");
		return T{};
	}

	template <typename INTEGRAL>
	traits::enable_if_t<traits::is_integral_v<INTEGRAL>, INTEGRAL>
		get_impl()
	{
		if (!initial_check(integral_type))
		{
			return INTEGRAL{ 0 };
		}
		++m_curr_arg;
		return get_integral<INTEGRAL>();
	}

	template <typename ENUM>
	traits::enable_if_t<traits::is_enum_v<ENUM>, ENUM>
		get_impl()
	{
		if (!initial_check(enum_type))
		{
			return ENUM::max_enum_value;
		}

		static constexpr auto max_enum_value = static_cast<traits::underlying_type_t<ENUM>>(ENUM::max_enum_value);
		const auto raw_val = get_integral<cell_type>();
		if (raw_val >= max_enum_value)
		{
			set_error();
			return ENUM::max_enum_value;
		}
		++m_curr_arg;
		return static_cast<ENUM>(raw_val);
	}

	template <typename POD_STRUCT>
	traits::enable_if_t<traits::is_pod_struct_v<POD_STRUCT>, POD_STRUCT>
		get_impl()
	{
		if (!initial_check(pod_struct_type))
		{
			return POD_STRUCT{};
		}

		const auto view = get_memory_view();
		if (view.size() != sizeof(POD_STRUCT))
		{
			set_error();
			return POD_STRUCT{};
		}

		POD_STRUCT arg{};
		if (!safe_memcpy(&arg, view.mem(), sizeof(POD_STRUCT)))
		{
			set_error();
		}
		++m_curr_arg;
		return arg;
	}

	template<>
	memory_view get_impl<memory_view>()
	{
		if (!initial_check(buffer_type))
		{
			return{ nullptr, 0UL };
		}
		++m_curr_arg;
		return get_memory_view();
	}

	template<>
	array_t get_impl<array_t>()
	{
		if (!initial_check(buffer_type))
		{
			return{};
		}
		++m_curr_arg;
		const auto view = get_memory_view();
		return{ view.mem(), view.size() + view.mem() };
	}

	template<>
	wstring_t get_impl<wstring_t>()
	{
		if (!initial_check(wstring_type))
		{
			return{};
		}

		++m_curr_arg;
		const auto view = get_memory_view();

		wstring_t ret(view.size() / sizeof(wchar_t), L'0');
		if (!safe_memcpy(&ret[0], view.mem(), view.size()))
		{
			set_error();
		}
		return ret;
	}

	template<>
	string_t get_impl<string_t>()
	{
		if (!initial_check(string_type))
		{
			return{};
		}
		++m_curr_arg;
		const auto view = get_memory_view();
		string_t ret(view.size(), '0');
		if (!safe_memcpy(&ret[0], view.mem(), view.size()))
		{
			set_error();
		}
		return ret;
	}

	bool initial_check(cell_type in_type);
	void set_error();
	cell_type read_argc();

	memory_view get_memory_view();

	template <typename T>
	T get_integral();

	static constexpr cell_type c_size_unknown{ 0ULL - 1 };

	cell_type m_offset{ 0 };
	cell_type m_curr_arg{ 0 };
	bool m_error{ false };

	const char* m_mem;
	const cell_type m_maxsize;
	const cell_type m_argc;
};

inline DeserializeIterator::DeserializeIterator(const void* mem)
	: m_mem(reinterpret_cast<const char*>(mem)), m_maxsize(c_size_unknown), m_argc(read_argc())
{}

inline DeserializeIterator::DeserializeIterator(const void* mem, size_t sz)
	: m_mem(reinterpret_cast<const char*>(mem)), m_maxsize(static_cast<cell_type>(sz)), m_argc(read_argc())
{}

inline memory_view DeserializeIterator::get_variable_buffer()
{
	if (!initial_check(variable_buffer_type))
	{
		return{ nullptr, 0UL };
	}

	++m_curr_arg;

	const auto real_max_size = get_integral<cell_type>();
	const auto view = get_memory_view();
	if (has_error())
	{
		return{ nullptr, 0UL };
	}

	if (has_error() || real_max_size < view.size())
	{
		set_error();
		return{ nullptr, 0UL };
	}

	return{ view.mem(), view.size() };
}

inline bool DeserializeIterator::finalize() const
{
	bool error = has_error();
	error |= (m_argc != m_curr_arg);

	if (m_maxsize != c_size_unknown)
	{
		error |= (m_offset > m_maxsize);
	}

	if (error)
	{
		assert(false);
	}

	return !error;
}

inline bool DeserializeIterator::has_error() const
{
	return m_error;
}

inline void DeserializeIterator::set_error()
{
	m_error = true;
}

inline bool DeserializeIterator::initial_check(cell_type in_type)
{
	if (has_error())
	{
		return false;
	}

	if (m_offset + cell_size + cell_size > m_maxsize)
	{
		set_error();
		return false;
	}

	const auto type = get_integral<cell_type>();
	if (type != in_type)
	{
		set_error();
		return false;
	}
	return true;
}

inline memory_view DeserializeIterator::get_memory_view()
{
	const auto buf_size = get_integral<cell_type>();
	if (has_error())
	{
		return{ nullptr, 0UL };
	}

	if (buf_size + m_offset > m_maxsize)
	{
		set_error();
		return{ nullptr, 0UL };
	}

	m_offset += buf_size;
	memory_view mv(m_mem + m_offset - buf_size, static_cast<size_t>(buf_size));
	return mv;
}

template <typename T>
T DeserializeIterator::get_integral()
{
	if (m_offset + cell_size > m_maxsize)
	{
		set_error();
		return true;
	}

	T ret{ 0 };
	__try {
#pragma warning( push ) // int -> bool conversion warning, we don't really care about performance penalty
#pragma warning( disable  : 4800 )
		ret = static_cast<T>(*reinterpret_cast<const cell_type*>(m_mem + m_offset));
#pragma warning( pop )
	} __except (1)
	{ //DPANIN fixme
		set_error();
	}
	if (has_error())
	{
		return 0;
	}

	m_offset += cell_size;
	return ret;
}

inline cell_type DeserializeIterator::read_argc()
{
	if (m_mem == nullptr || m_maxsize < cell_size)
	{
		set_error();
		return 0;
	}

	return get_integral<cell_type>();
}
