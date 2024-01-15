#pragma once

namespace loquat::math
{
	template <typename T>
		requires std::integral<T> || std::floating_point<T>
	constexpr T square(T value) noexcept
	{
		return value * value;
	}
}