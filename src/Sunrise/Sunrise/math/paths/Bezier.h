#pragma once

#include "srpch.h"

namespace sunrise::math
{
	namespace Bezier {
		template<typename p, typename time>
		SUNRISE_API p evaluateQuadratic(p a, p b, p c, time t);


		template<typename p, typename time>
		SUNRISE_API p evaluateCubic(p a, p b, p c, p d, time t);


		template<typename p, typename time>
		SUNRISE_API p lerp(p a, p b, time t);


		template<typename p, typename time>
		p evaluateQuadratic(p a, p b, p c, time t)
		{
			auto p0 = lerp(a, b, t);
			auto p1 = lerp(b, c, t);
			return lerp(p0, p1, t);
		}

		template<typename p, typename time>
		p evaluateCubic(p a, p b, p c, p d, time t)
		{
			auto p0 = evaluateQuadratic(a, b, c, t);
			auto p1 = evaluateQuadratic(b, c, d, t);
			return lerp(p0, p1, t);
		}


		template<typename p, typename time>
		p lerp(p a, p b, time t)
		{
			return  a * (1 - t) + b * t;
		}
	};
};

