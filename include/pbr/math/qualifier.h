#pragma once

namespace loquat
{
	/// <summary>
	/// Used to specify packing, alignment, and precision for other types.
	/// </summary>
	enum Qualifier
	{
		packed_high_precision,
		packed_medium_precision,
		packed_low_precision,

		aligned_high_precision,
		aligned_medium_precision,
		aligned_low_precision,

		default_precision = packed_high_precision
	};
}