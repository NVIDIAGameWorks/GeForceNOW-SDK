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
#include <string>
#include <tuple>
#include <lpc_pipe.h>

namespace SampleService
{
	namespace Transport
	{
		template <size_t IDX, size_t MAX, bool overflow = IDX >= MAX>
		struct write_to_tuple;

		template <size_t IDX, size_t MAX>
		struct write_to_tuple<IDX, MAX, false>
		{
			template <typename TUPLE>
			static void call(DeserializeIterator& it, TUPLE& tpl)
			{
				using TP = std::decay_t<std::tuple_element_t<IDX, TUPLE>>;
				std::get<IDX>(tpl) = it.get<TP>();
			}
		};

		template <size_t IDX, size_t MAX>
		struct write_to_tuple<IDX, MAX, true>
		{
			template <typename TUPLE>
			static void call(const DeserializeIterator&, const TUPLE&)
			{}
		};

		template <typename ... RETVALS>
		void deserializer_to_tuple_check_finalize(DeserializeIterator& it, std::tuple<status, RETVALS...>& tpl)
		{
			static constexpr size_t tpl_sz = std::tuple_size<std::decay_t<decltype(tpl)>>::value;
			static_assert(tpl_sz < 10, "More than 10 elements? Add more calls to write_to_tuple, then change me!");

			auto& status = std::get<0>(tpl);

			write_to_tuple<0, tpl_sz>::call(it, tpl);
			if (status != status::success)
			{
				return;
			}

			write_to_tuple<1, tpl_sz>::call(it, tpl);
			write_to_tuple<2, tpl_sz>::call(it, tpl);
			write_to_tuple<3, tpl_sz>::call(it, tpl);
			write_to_tuple<4, tpl_sz>::call(it, tpl);
			write_to_tuple<5, tpl_sz>::call(it, tpl);
			write_to_tuple<6, tpl_sz>::call(it, tpl);
			write_to_tuple<7, tpl_sz>::call(it, tpl);
			write_to_tuple<8, tpl_sz>::call(it, tpl);
			write_to_tuple<9, tpl_sz>::call(it, tpl);

			if (!it.finalize())
			{
				status = status::deserialization_error;
			}
		}

		template <typename ... RETVALS, typename ... ARGS>
		std::tuple<status, RETVALS...> send_impl(LPCPipeClient& pipe, size_t timeout, command cmd, const ARGS&... args)
		{
			std::tuple<status, RETVALS...> ret;

			if (!pipe.isConnected())
			{
				if (!pipe.connect(timeout))
				{
					std::get<0>(ret) = status::failed_to_create_pipe;
					return ret;
				}
			}

			const auto sender = pipe.getSender();
			DeserializeIterator deserializer = sender.send(cmd, args...);
			deserializer_to_tuple_check_finalize(deserializer, ret);
			return ret;
		}
	}
}
