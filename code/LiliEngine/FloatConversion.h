#pragma once

#include "CoreMath.h"

// TODO: перенести сюда связанное с float

namespace math
{
	// returns true if the distance between f0 and f1 is smaller than epsilon
	inline bool CompareApproximately(float f0, float f1, float epsilon = 0.000001F)
	{
		const float dist = Abs(f0 - f1);
		return dist < epsilon;
	}
} // namespace math