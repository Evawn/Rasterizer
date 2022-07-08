/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Some geometric routines to save implementation time.
 * 
 * Borrowed from Mitsuba 0.5.0 by Wenzel Jakob
 * https://www.mitsuba-renderer.org
 */

#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "common.hpp"

namespace RTUtil {

    using Point2 = Eigen::Vector2f;
    using Vector3 = Eigen::Vector3f;

	RTUTIL_EXPORT Point2 squareToUniformDiskConcentric(const Point2 &sample);
	RTUTIL_EXPORT Vector3 squareToCosineHemisphere(const Point2 &sample);
	RTUTIL_EXPORT Vector3 nonParallel(const Vector3 &v);

}

namespace math {

#if defined(_GNU_SOURCE)
    inline void sincos(float theta, float *sin, float *cos) {
        ::sincosf(theta, sin, cos);
    }

#else
    inline void sincos(float theta, float *_sin, float *_cos) {
        *_sin = sinf(theta);
        *_cos = cosf(theta);
    }
#endif

    /// Square root variant that gracefully handles arguments < 0 that are due to roundoff errors
    inline float safe_sqrt(float value) {
        return std::sqrt(std::max(0.0f, value));
    }

    // Borrowed from https://en.cppreference.com/w/cpp/algorithm/clamp
    template<class T>
    constexpr const T& clamp( const T& v, const T& lo, const T& hi )
    {
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

}
