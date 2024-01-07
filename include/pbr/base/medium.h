#pragma once

#include "pbr/util/tagged_pointer.h"

namespace loquat
{
	class HomogeneousMedium;
	class GridMedium;
	class RGBGridMedium;
	class CloudMedium;
	class NanoVDBMedium;

	class Medium : public TaggedPointer<HomogeneousMedium, GridMedium, 
		RGBGridMedium, CloudMedium, NanoVDBMedium>
	{
	public:
		using TaggedPointer::TaggedPointer;

		std::string to_string() const;
		bool is_emissive() const;
	};

	class MediumInterface
	{
	public:
		Medium inside;
		Medium outside;
	};
}