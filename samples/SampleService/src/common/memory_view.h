/*
* Copyright (c) 2016-2018, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once
#include "serialize_common.h"

class memory_view
{
public:
	memory_view(const void* mem, size_t size_in_bytes);
	memory_view(const void* mem, unsigned long size_in_bytes);

	memory_view(const memory_view&) = default;
	memory_view& operator= (const memory_view&) = default;

	unsigned long size() const;
	const char* mem() const;
	char* mem();

private:
	char* m_mem;
	unsigned long m_size;
};

inline memory_view::memory_view(const void* mem, size_t size_in_bytes)
	: m_mem(reinterpret_cast<char*>(const_cast<void*>(mem))), m_size(static_cast<unsigned long>(size_in_bytes))
{}

inline memory_view::memory_view(const void* mem, unsigned long size_in_bytes)
	: m_mem(reinterpret_cast<char*>(const_cast<void*>(mem))), m_size(size_in_bytes)
{}

inline unsigned long memory_view::size() const
{
	return m_size;
}

inline const char* memory_view::mem() const
{
	return m_mem;
}

inline char* memory_view::mem()
{
	return m_mem;
}

class variable_buffer
{
	friend class serialize_iterator;
	variable_buffer(const variable_buffer&) = delete;
	variable_buffer& operator=(const variable_buffer&) = delete;
	variable_buffer& operator=(variable_buffer&&) = delete;
public:
	variable_buffer(variable_buffer&& rhv) noexcept :
		m_mem(rhv.m_mem), m_max_size(rhv.m_max_size), m_real_size(rhv.m_real_size), m_real_size_of_buffer(rhv.m_real_size_of_buffer)
	{}

	unsigned remaining_bytes() const
	{
		return static_cast<unsigned> (
			m_real_size != nullptr ? m_max_size - *m_real_size : 0
			);
	}

	// current filled size, not max size
	unsigned size() const
	{
		return static_cast<unsigned> (
			m_real_size != nullptr ? *m_real_size : 0
			);
	}

	char* mem() const
	{
		return m_mem;
	}

	void adjust_pointer(unsigned offset)
	{
		if (m_real_size == nullptr)
		{
			return;
		}

		if (offset + *m_real_size > m_max_size)
		{
			const unsigned long diff = (unsigned long) m_max_size - (unsigned long)*m_real_size;
			(*m_real_size) += diff;
			m_mem += diff;
			(*m_real_size_of_buffer) += diff;
			return;
		}

		(*m_real_size) += offset;
		m_mem += offset;
		(*m_real_size_of_buffer) += offset;
	}

	variable_buffer() :
		m_mem(nullptr), m_max_size(0), m_real_size(nullptr), m_real_size_of_buffer(nullptr)
	{}

	variable_buffer(char* mem, cell_type max_size, cell_type* const real_size, unsigned long* real_size_of_buffer) :
		m_mem(mem), m_max_size(max_size), m_real_size(real_size), m_real_size_of_buffer(real_size_of_buffer)
	{
		*m_real_size = 0;
	}

private:
	char* m_mem;
	const cell_type m_max_size;
	cell_type* const m_real_size;
	unsigned long* const m_real_size_of_buffer;
};
