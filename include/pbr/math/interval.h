// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>

#include "debug/logger.h"

namespace loquat
{
	class Interval
	{
	public:
		constexpr Interval() = default;
		LOQUAT_CPU_GPU
		constexpr explicit Interval(const Float value) noexcept
			: low{ value }
			, high{ value }
		{}
		LOQUAT_CPU_GPU
		constexpr Interval(const Float low, const Float high) noexcept
			: low{ std::min(low, high) }
			, high{ std::max(low, high) }
		{}
		LOQUAT_CPU_GPU
		Interval& operator=(const Float value)
		{
			low = value;
			high = value;
			return *this;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		static Interval from_value_and_error(const Float value, 
			const Float error) noexcept
		{
			Interval result;

			if (error == 0)
			{
				result.low = value;
				result.high = value;
			}
			else
			{
				result.low = sub_round_down(value, error);
				result.high = add_round_up(value, error);
			}
			return result;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		constexpr Float lower_bound() const noexcept
		{
			return low;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		constexpr Float upper_bound() const noexcept
		{
			return high;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		constexpr Float midpoint() const noexcept
		{
			return (low + high) / 2;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		constexpr Float width() const noexcept
		{
			return (high - low);
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Float operator[](int i) const
		{
			LOG_ASSERT(i == 0 || i == 1 && "Invalid range on interval");
			return (i == 0) ? low : high;
		}

		LOQUAT_CPU_GPU
		explicit operator Float() const noexcept
		{
			return midpoint();
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool exactly(Float value) const noexcept
		{
			return low == value && high == value;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool operator==(Float value) const noexcept
		{
			return exactly(value);
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interval operator-() const noexcept
		{
			return { -low, -high };
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interval operator+(Interval i) const noexcept
		{
			return {
				add_round_down(low, i.low), 
				add_round_up(high, i.high)
			};
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interval operator-(Interval i) const noexcept
		{
			return {
				sub_round_down(low, i.low),
				sub_round_up(high, i.high)
			};
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interval operator*(Interval i) const noexcept
		{
			Float lp[4] = { 
				mul_round_down(low, i.low),
				mul_round_down(high, i.low),
				mul_round_down(low, i.high),
				mul_round_down(high, i.high)
			};
			Float hp[4] = {
				mul_round_up(low, i.low),
				mul_round_up(high, i.low),
				mul_round_up(low, i.high),
				mul_round_up(high, i.high)
			};
			return {
				std::min({lp[0], lp[1], lp[2], lp[3]}),
				std::max({hp[0], hp[1], hp[2], hp[3]})
			};
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		Interval operator/(Interval i) const noexcept;

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool operator==(Interval i) const noexcept
		{
			return low == i.low && high == i.high;
		}

		[[nodiscard]]
		LOQUAT_CPU_GPU
		bool operator!=(Float f) const noexcept
		{
			return f < low || f > high;
		}

		[[nodiscard]]
		std::string to_string() const;

		LOQUAT_CPU_GPU
		Interval& operator+=(Interval i) noexcept
		{
			*this = Interval(*this + i);
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator-=(Interval i) noexcept
		{
			*this = Interval(*this - i);
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator*=(Interval i) noexcept
		{
			*this = Interval(*this * i);
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator/=(Interval i) noexcept
		{
			*this = Interval(*this / i);
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator+=(Float f) noexcept
		{
			this->low += f;
			this->high += f;
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator-=(Float f) noexcept
		{
			this->low -= f;
			this->high -= f;
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator*=(Float f) noexcept
		{
			if (f > 0)
			{
				*this = { 
					mul_round_down(low, f),
					mul_round_up(high, f)
				};
			}
			else
			{
				*this = {
					mul_round_down(high, f),
					mul_round_up(low, f)
				};
			}
			return *this;
		}

		LOQUAT_CPU_GPU
		Interval& operator/=(Float f) noexcept
		{
			if (f > 0)
			{
				*this = {
					div_round_down(low, f),
					div_round_up(high, f)
				};
			}
			else
			{
				*this = {
					div_round_down(high, f),
					div_round_up(low, f)
				};
			}
			return *this;
		}

#ifndef LOQUAT_IS_GPU_CODE
		static const Interval Pi;
#endif

	private:
		//friend struct SOA<Interval>;
		//TODO(ches) handle SOA generation
		Float low;
		Float high;
	};
}
