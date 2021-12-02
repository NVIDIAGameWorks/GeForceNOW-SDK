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
#include "deserialize_buffer.h"
#include "memory_view.h"

template <typename T>
traits::enable_if_t<traits::is_unspecified_v<T>, cell_type>
calc_elem_size(const T&)
{
	static_assert(false, "Non-serializable type: only integrals, enums, POD structs, wstring_t, array_t and buffer are supported");
	return 0ULL;
}

template <typename T>
traits::enable_if_t<traits::is_unspecified_v<T>, bool>
serialize_elem(char*&, cell_type&, const T&)
{
	static_assert(false, "Non-serializable type: only integrals, enums, POD structs, wstring_t, array_t and buffer are supported");
	return false;
}

// === INTEGRAL + ENUM
template <typename T>
traits::enable_if_t<traits::is_integral_v<T> || traits::is_enum_v<T>, cell_type>
calc_elem_size(const T&)
{
	return estimated_size<T>;
}

template <typename T>
traits::enable_if_t<traits::is_integral_v<T>, bool>
serialize_elem(char*& mem, cell_type& remaining_size, const T& integral_arg)
{
	memcpy_s(mem, remaining_size, &integral_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type cell = integral_arg;
	memcpy_s(mem, remaining_size, &cell, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;
	return true;
}

template <typename T>
traits::enable_if_t<traits::is_enum_v<T>, bool>
serialize_elem(char*& mem, cell_type& remaining_size, const T& enum_arg)
{
	memcpy_s(mem, remaining_size, &enum_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type cell = static_cast<traits::underlying_type_t<T>>(enum_arg);
	memcpy_s(mem, remaining_size, &cell, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;
	return true;
}

template <typename T>
traits::enable_if_t<traits::is_enum_v<T>, T*>
serialize_elem_reserve(char*& mem, cell_type& remaining_size, const T& enum_arg)
{
	memcpy_s(mem, remaining_size, &enum_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type cell = static_cast<traits::underlying_type_t<T>>(enum_arg);
	memcpy_s(mem, remaining_size, &cell, cell_size);

	T* ptr = reinterpret_cast<T*>(mem);

	mem += cell_size;
	remaining_size -= cell_size;
	return ptr;
}

// === POD_STRUCT
template <typename T>
traits::enable_if_t<traits::is_pod_struct_v<T>, cell_type>
calc_elem_size(const T&)
{
	return estimated_size<T>;
}

template <typename T>
traits::enable_if_t<traits::is_pod_struct_v<T>, bool>
serialize_elem(char*& mem, cell_type& remaining_size, const T& pod_struct_arg)
{
	memcpy_s(mem, remaining_size, &pod_struct_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type buf_size{ sizeof(T) };
	memcpy_s(mem, remaining_size, &buf_size, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	memcpy_s(mem, remaining_size, &pod_struct_arg, buf_size);
	mem += buf_size;
	remaining_size -= buf_size;
	return true;
}

// === MEMORY_VIEW
inline cell_type calc_elem_size(const memory_view& arg)
{
	return cell_size * 2 + arg.size();
}

inline bool serialize_elem(char*& mem, cell_type& remaining_size, const memory_view& buf)
{
	memcpy_s(mem, remaining_size, &buffer_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type buf_size{ buf.size() };
	memcpy_s(mem, remaining_size, &buf_size, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	memcpy_s(mem, remaining_size, buf.mem(), buf_size);
	mem += buf_size;
	remaining_size -= buf_size;
	return true;
}

// === STRING
inline cell_type calc_elem_size(const string_t& arg)
{
	return cell_size * 2 + arg.size();
}

inline bool serialize_elem(char*& mem, cell_type& remaining_size, const string_t& str)
{
	memcpy_s(mem, remaining_size, &string_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type buf_size{ str.size() };
	memcpy_s(mem, remaining_size, &buf_size, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	memcpy_s(mem, remaining_size, str.c_str(), buf_size);
	mem += buf_size;
	remaining_size -= buf_size;
	return true;
}

// === WSTRING
inline cell_type calc_elem_size(const wstring_t& arg)
{
	return cell_size * 2 + arg.size() * sizeof(wchar_t);
}

inline bool serialize_elem(char*& mem, cell_type& remaining_size, const wstring_t& wstr)
{
	memcpy_s(mem, remaining_size, &wstring_type, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	const cell_type buf_size{ wstr.size() * sizeof(wchar_t) };
	memcpy_s(mem, remaining_size, &buf_size, cell_size);
	mem += cell_size;
	remaining_size -= cell_size;

	memcpy_s(mem, remaining_size, wstr.data(), buf_size);
	mem += buf_size;
	remaining_size -= buf_size;
	return true;
}

inline cell_type count_mem()
{
	return 0;
}

template <typename T, typename ... ARGS>
cell_type count_mem(const T& head, const ARGS&... tail)
{
	return calc_elem_size(head) + count_mem(tail...);
}

inline void serialize_impl(char*, cell_type)
{ /* end of variadic recursion */}

template <typename T, typename ... ARGS>
void serialize_impl(char*& ret, cell_type& remaining_size, const T& head, const ARGS&... tail)
{
	serialize_elem(ret, remaining_size, head);
	serialize_impl(ret, remaining_size, tail...);
}

template <typename ... ARGS>
array_t serialize(const ARGS&... args)
{
	const cell_type argc = sizeof...(args);
	cell_type mem_size = count_mem(args...) + cell_size;

	array_t container(mem_size);
	memcpy_s(&container[0], cell_size, &argc, cell_size);

	auto mem_ptr = &container[cell_size];

	mem_size -= cell_size;
	serialize_impl(mem_ptr, mem_size, args...);
	return container;
}

template <typename ... ARGS>
size_t serialize_to(char* buffer, size_t size, const ARGS&... args)
{
	const cell_type argc = sizeof...(args);
	cell_type mem_size = count_mem(args...) + cell_size;
	if (mem_size > size)
	{
		return 0;
	}

	memcpy_s(&buffer[0], cell_size, &argc, cell_size);

	auto mem_ptr = &buffer[cell_size];

	auto remaining_size = mem_size - cell_size;
	serialize_impl(mem_ptr, remaining_size, args...);
	return mem_size;
}

class SerializeIterator
{
	SerializeIterator(const SerializeIterator&) = delete;
	SerializeIterator(SerializeIterator&&) = delete;
	SerializeIterator& operator=(const SerializeIterator&) = delete;
	SerializeIterator& operator=(SerializeIterator&&) = delete;
public:
	SerializeIterator(void* mem, size_t max_size, unsigned long* real_size) :
		m_mem(reinterpret_cast<char*>(mem) + sizeof(size_t)),
		m_argc(reinterpret_cast<cell_type*>(mem)),
		m_maxsize(static_cast<cell_type>(max_size)),
		m_realsize(real_size)
	{
		*m_argc = 0;
		*m_realsize = cell_size; // for ARGC
	}

	// STATUS_SUCCESS on everything good
	// STATUS_BUFFER_TOO_SMALL on overflow
	// STATUS_DATA_ERROR on empty buffer
	unsigned long valid() const
	{
		return 0; //hack, fixme, DPANIN
	}

	template <typename T>
	void put(const T& arg)
	{
		const auto elem_sz = static_cast<unsigned long>(calc_elem_size(arg));
		*m_realsize += elem_sz;

		if (!m_write_error && m_maxsize >= *m_realsize)
		{
			// HACK, DPANIN
			cell_type fake = 1234567ULL;

			serialize_elem(m_mem, fake, arg);
			++(*m_argc);
		}
	}

	// serialize-reserve operation, basically:
	// * reserves a cell for given datatype
	// * sets a default value
	// * returns a pointer to the value
	// should be considered unsafe
	template <typename T>
	T* reserve(const T& arg)
	{
		const auto elem_sz = static_cast<unsigned long>(calc_elem_size(arg));
		*m_realsize += elem_sz;

		if (!m_write_error && m_maxsize >= *m_realsize)
		{
			// HACK, DPANIN
			cell_type fake = 1234567ULL;

			T* ptr = serialize_elem_reserve(m_mem, fake, arg);
			++(*m_argc);

			return ptr;
		}

		return nullptr;
	}

	// warning, nothing can be serialized after this
	variable_buffer put_variable_buffer()
	{
		constexpr auto control_block_size = cell_size * 3;

		if (*m_realsize + control_block_size > m_maxsize)
		{
			m_write_error = true;
			return{};
		}

		cell_type* const type = reinterpret_cast<cell_type*>(m_mem);
		*type = variable_buffer_type;

		cell_type* const max_buf_size = reinterpret_cast<cell_type*>(m_mem + cell_size);
		*max_buf_size = m_maxsize - *m_realsize - control_block_size;

		cell_type* const real_buf_size = reinterpret_cast<cell_type*>(m_mem + cell_size + cell_size);

		*m_realsize += control_block_size;
		++(*m_argc);

		return{ m_mem + control_block_size, *max_buf_size, real_buf_size, m_realsize };
	}

private:
	char* m_mem;
	cell_type* m_argc;
	const cell_type m_maxsize;
	unsigned long* const m_realsize;

	bool m_write_error{ false };
};