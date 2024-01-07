#pragma once
// Modified from pbrt
// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <type_traits>

#include "debug/logger.h"

namespace loquat
{
#pragma region Template processing
	template <typename... Ts>
	struct TypePack
	{
		static constexpr size_t count = sizeof...(Ts);
	};

	template <typename T, typename... Ts>
	struct IndexOf
	{
		static constexpr int count = 0;
		static_assert(!std::is_same_v<T, T>, "Type not present in TypePack");
	};

	template <typename T, typename... Ts>
	struct IndexOf<T, TypePack<T, Ts...>>
	{
		static constexpr int count = 0;
	};

	template <typename T, typename U, typename... Ts>
	struct IndexOf<T, TypePack<U, Ts...>>
	{
		static constexpr int count = 1 + IndexOf<T, TypePack<Ts...>>::count;
	};

	template <typename T, typename... Ts>
	struct HasType
	{
		static constexpr bool value = false;
	};

	template <typename T, typename Tfirst, typename... Ts>
	struct HasType<T, TypePack<Tfirst, Ts...>>
	{
		static constexpr bool value =
			(std::is_same_v<T, Tfirst> || HasType<T, TypePack<Ts...>>::value);
	};

	template <typename T>
	struct GetFirst {};
	template <typename T, typename... Ts>
	struct GetFirst<TypePack<T, Ts...>>
	{
		using type = T;
	};

	template <typename T>
	struct RemoveFirst {};
	template <typename T, typename... Ts>
	struct RemoveFirst<TypePack<T, Ts...>>
	{
		using type = TypePack<Ts...>;
	};

	template <int index, typename T, typename... Ts>
	struct RemoveFirstN;
	template <int index, typename T, typename... Ts>
	struct RemoveFirstN<index, TypePack<T, Ts...>>
	{
		using type = typename RemoveFirstN<index - 1, TypePack<Ts...>>::type;
	};

	template <typename T, typename... Ts>
	struct RemoveFirstN<0, TypePack<T, Ts...>>
	{
		using type = TypePack<T, Ts...>;
	};

	template <typename... Ts>
	struct Prepend;
	template <typename T, typename... Ts>
	struct Prepend<T, TypePack<Ts...>>
	{
		using type = TypePack<T, Ts...>;
	};
	template <typename... Ts>
	struct Prepend<void, TypePack<Ts...>>
	{
		using type = TypePack<Ts...>;
	};

	template <int index, typename T, typename... Ts>
	struct TakeFirstN;
	template <int index, typename T, typename... Ts>
	struct TakeFirstN<index, TypePack<T, Ts...>>
	{
		using type = typename Prepend<T, 
			typename TakeFirstN<index - 1, TypePack<Ts...>>::type>::type;
	};
	template <typename T, typename... Ts>
	struct TakeFirstN<1, TypePack<T, Ts...>>
	{
		using type = TypePack<T>;
	};

	template <template <typename> class M, typename... Ts>
	struct MapType;
	template <template <typename> class M, typename T>
	struct MapType<M, TypePack<T>>
	{
		using type = TypePack<M<T>>;
	};

	template <template <typename> class M, typename T, typename... Ts>
	struct MapType<M, TypePack<T, Ts...>>
	{
		using type = typename Prepend<M<T>, 
			typename MapType<M, TypePack<Ts...>>::type>::type;
	};

	template <typename Base, typename... Ts>
	inline constexpr bool all_inherit_from(TypePack<Ts...>);

	template <typename Base>
	inline constexpr bool all_inherit_from(TypePack<>)
	{
		return true;
	}

	template <typename Base, typename T, typename... Ts>
	inline constexpr bool all_inherit_from(TypePack<T, Ts...>)
	{
		return std::is_base_of_v<Base, T>
			&& all_inherit_from<Base>(TypePack<Ts...>());
	}

	template <typename F, typename... Ts>
	void for_each_type(F func, TypePack<Ts...>);

	template <typename F, typename T, typename... Ts>
	void for_each_type(F func, TypePack<T, Ts...>)
	{
		func.template operator()<T>();
		for_each_type(func, TypePack<Ts...>());
	}

	template <typename F>
	void for_each_type(F func, TypePack<>) {}
#pragma endregion

	template <typename... Ts>
	class TaggedPointer
	{
	public:
		using Types = TypePack<Ts...>;

		TaggedPointer() = default;

		template <typename T>
		TaggedPointer(T* pointer) {
			uint64_t iptr = reinterpret_cast<uint64_t>(pointer);
			LOG_ASSERT(iptr & POINTER_MASK == iptr
				&& "Our pointer tagging is breaking the pointers");
			constexpr unsigned int type = TypeIndex<T>();
			bits = iptr | (static_cast<uint64_t>(type) << POINTER_BITS);
		}
		TaggedPointer(std::nullptr_t np)
		{}
		TaggedPointer(const TaggedPointer& t)
			: bits{ t.bits }
		{}
		TaggedPointer& operator=(const TaggedPointer& t)
		{
			bits = t.bits;
			return *this;
		}

		template <typename T>
		static constexpr unsigned int type_index()
		{
			using Tp = typename std::remove_cv_t<T>;
			if constexpr (std::is_same_v<Tp, std::nullptr_t>)
			{
				return 0;
			}
			else
			{
				return 1 + loquat::IndexOf<Tp, Types>::count;
			}
		}

		unsigned int tag() const
		{
			return ((bits & TAG_MASK) >> POINTER_BITS);
		}

		template <typename T>
		bool is() const
		{
			return tag() == TypeIndex<T>();
		}

		static constexpr unsigned int max_tag()
		{
			return sizeof...(Ts);
		}

		static constexpr unsigned int num_tags()
		{
			return max_tag() + 1;
		}

		explicit operator bool() const
		{
			return (bits & POINTER_MASK) != 0;
		}

		bool operator<(const TaggedPointer& tp) const
		{
			return bits < tp.bits;
		}

		template <typename T>
		T* cast()
		{
			LOG_ASSERT(is<T>() && "Casting tagged pointer to the wrong type");
			return reinterpret_cast<T*>(pointer());
		}

		template <typename T>
		const T* cast() const
		{
			LOG_ASSERT(is<T>() && "Casting tagged pointer to the wrong type");
			return reinterpret_cast<const T*>(pointer());
		}

		template <typename T>
		T* cast_or_nullptr()
		{
			if (is<T>())
			{
				return reinterpret_cast<T*>(pointer());
			}
			return nullptr;
		}

		template <typename T>
		const T* cast_or_nullptr() const
		{
			if (is<T>())
			{
				return reinterpret_cast<const T*>(pointer());
			}
			return nullptr;
		}

		std::string to_string() const
		{
			return std::format("[ TaggedPointer ptr: {:#x} tag: {:d} ]",
				pointer(), tag());
		}

		bool operator==(const TaggedPointer& tp) const
		{
			return bits == tp.bits;
		}

		bool operator!=(const TaggedPointer& tp) const
		{
			return bits != tp.bits;
		}

		void* pointer()
		{
			return reinterpret_cast<void*>(bits & POINTER_MASK);
		}

		const void* pointer() const
		{
			return reinterpret_cast<const void*>(bits & POINTER_MASK);
		}

	private:
		static_assert(sizeof...(Ts) < std::numeric_limits<uint16_t>::max(),
			"Too many types");
		static_assert(sizeof(uintptr_t) <= sizeof(uint64_t),
			"Expected pointer size to be <= 64 bits");

		static constexpr int POINTER_BITS = 57;
		static constexpr int TAG_BITS = 64 - POINTER_BITS;
		static constexpr uint64_t TAG_MASK =
			((1ull << TAG_BITS) - 1) << POINTER_BITS;
		static constexpr uint64_t POINTER_MASK = ~TAG_MASK;
		uint64_t bits = 0;
	};
}
