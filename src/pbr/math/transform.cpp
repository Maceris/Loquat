// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#include "pbr/math/transform.h"

namespace loquat
{
	[[nodiscard]]
	Point3fi Transform::operator()(const Point3fi& point) const noexcept
	{
		Vec4f point4 = Vec4f(point.to_vec(), 1);
		Vec4f transformed = matrix * point4;

		Vec3f error;

		if (point.is_exact())
		{
			Vec4f gamma{ loquat::gamma(3) };
			error = gamma * glm::abs(transformed);
		}
		else
		{
			Vec4f error_input = Vec4f(point.error(), 1);
			Vec4f gamma{ loquat::gamma(3) + 1 };
			error = gamma * glm::abs(transformed);
		}

		if (transformed.w == 1)
		{
			return Point3fi(transformed, error);
		}
		else
		{
			return Point3fi(transformed / transformed.w, error);
		}

	}

}