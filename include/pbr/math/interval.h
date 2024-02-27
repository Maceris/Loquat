// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <algorithm>

#include "main/loquat.h"

namespace loquat
{
	class Interval
	{
	public:
		constexpr Interval() = default;
		constexpr explicit Interval(const Float value) noexcept
			: range{ value, value }
		{}
		constexpr Interval(const Float low, const Float high) noexcept
			: range{ std::min(low, high), std::max(low, high) }
		{}
		Interval& operator=(const Float value)
		{
			range = { value, value };
			return *this;
		}

		[[nodiscard]]
		static Interval from_value_and_error(const Float value, 
			const Float error) noexcept
		{
			Interval result;

			if (error == 0)
			{
				result.range = { value, value };
			}
			else
			{
				result.range.x = sub_round_down(value, error);
				result.range.y = add_round_up(value, error);
			}
			return result;
		}

		[[nodiscard]]
		constexpr Float low() const noexcept
		{
			return range.x;
		}

		[[nodiscard]]
		constexpr Float high() const noexcept
		{
			return range.y;
		}

		[[nodiscard]]
		constexpr Float midpoint() const noexcept
		{
			return (range.x + range.y) / 2;
		}

		[[nodiscard]]
		constexpr Float width() const noexcept
		{
			return (range.y - range.x);
		}

		[[nodiscard]]
		Float operator[](int i) const
		{
			LOG_ASSERT(i == 0 || i == 1 && "Invalid range on interval");
			return range[i];
		}

		explicit operator Float() const noexcept
		{
			return midpoint();
		}

		[[nodiscard]]
		bool exactly(Float value) const noexcept
		{
			return range.x == value && range.y == value;
		}

		[[nodiscard]]
		bool operator==(Float value) const noexcept
		{
			return exactly(value);
		}

		[[nodiscard]]
		Interval operator-() const noexcept
		{
			return { -range.x, -range.y };
		}

		[[nodiscard]]
		Interval operator+(Interval i) const noexcept
		{
			return {
				add_round_down(low(),i.low()), 
				add_round_up(high(), i.high())
			};
		}

		[[nodiscard]]
		Interval operator-(Interval i) const noexcept
		{
			return {
				sub_round_down(low(), i.low()),
				sub_round_up(high(), i.high())
			};
		}

		[[nodiscard]]
		Interval operator*(Interval i) const noexcept
		{
			Float lp[4] = { 
				mul_round_down(low(), i.low()),
				mul_round_down(high(), i.low()),
				mul_round_down(low(), i.high()),
				mul_round_down(high(), i.high())
			};
			Float hp[4] = {
				mul_round_up(low(), i.low()),
				mul_round_up(high(), i.low()),
				mul_round_up(low(), i.high()),
				mul_round_up(high(), i.high())
			};
			return {
				std::min({lp[0], lp[1], lp[2], lp[3]}),
				std::max({hp[0], hp[1], hp[2], hp[3]})
			};
		}

		[[nodiscard]]
		Interval operator/(Interval i) const noexcept;

		[[nodiscard]]
		bool operator==(Interval i) const noexcept
		{
			return low() == i.low() && high() == i.high();
		}

		[[nodiscard]]
		bool operator!=(Float f) const noexcept
		{
			return f < range.x || f > range.y;
		}

		[[nodiscard]]
		std::string to_string() const;

		Interval& operator+=(Interval i) noexcept
		{
			this->range += i.range;
			return *this;
		}

		Interval& operator-=(Interval i) noexcept
		{
			this->range -= i.range;
			return *this;
		}

		Interval& operator*=(Interval i) noexcept
		{
			this->range *= i.range;
			return *this;
		}

		Interval& operator/=(Interval i) noexcept
		{
			this->range /= i.range;
			return *this;
		}

		Interval& operator+=(Float f) noexcept
		{
			this->range += Vec2f{ f, f };
			return *this;
		}

		Interval& operator-=(Float f) noexcept
		{
			this->range -= Vec2f{ f, f };
			return *this;
		}

		Interval& operator*=(Float f) noexcept
		{
			if (f > 0)
			{
				this->range = { 
					mul_round_down(low(), f),
					mul_round_up(high(), f)
				};
			}
			else
			{
				this->range = {
					mul_round_down(high(), f),
					mul_round_up(low(), f)
				};
			}
			return *this;
		}

		Interval& operator/=(Float f) noexcept
		{
			if (f > 0)
			{
				this->range = {
					div_round_down(low(), f),
					div_round_up(high(), f)
				};
			}
			else
			{
				this->range = {
					div_round_down(high(), f),
					div_round_up(low(), f)
				};
			}
			return *this;
		}

	private:
		Vec2f range;
	};
}
