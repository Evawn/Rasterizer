/*
 * Cornell CS5625
 * RTUtil library
 *
 * Common definitions for build settings.
 *
 * Author: srm, Spring 2020
 */

#pragma once

#include <memory>
#include <iostream>

#if defined(_WIN32)
#  if defined(RTUTIL_BUILD)
#    define RTUTIL_EXPORT __declspec(dllexport)
#  else
#    define RTUTIL_EXPORT __declspec(dllimport)
#  endif
#elif defined(RTUTIL_BUILD)
#  define RTUTIL_EXPORT __attribute__ ((visibility("default")))
#else
#  define RTUTIL_EXPORT
#endif
