// Sky.hpp
//
// Cornell CS5625 Spring 2020
//
// author: Steve Marschner, April 2020
//

#pragma once

#include "common.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <GLWrap/Program.hpp>

namespace RTUtil {

    // A simple implementation of Preetham sky model that works with
    // the fragment shader code in sunsky.fs to evaluate sun and sky
    // radiance based on sun position and atmospheric conditions.
    //
    // To use it, link sunsky.fs into a shader program, create a
    // Sky instance with the parameters you want, and call
    // setUniforms with the shader program before you draw.  Then
    // in the fragment shader code you can call
    //     sunskyRadiance(worldDir)
    // to get an RGB color for the sky in a given world-space
    // direction.

    class RTUTIL_EXPORT Sky {
    public:

        Sky(float theta_sun, float turbidity)
        : theta_sun(theta_sun), turbidity(turbidity) { 
            init();
        }

        void setThetaSun(float theta_sun) {
            this->theta_sun = theta_sun;
        }

        void setTurbidity(float turbidity) {
            this->turbidity = turbidity;
        }

        // Sets the uniforms that are required for 
        // the sunskyRadiance shader function to operate.
        void setUniforms(GLWrap::Program &);

    private:

        float theta_sun, turbidity;

        // Matrices from the appendix of the paper
        // that are needed to compute the parameters to
        // the sky model.
        Eigen::Matrix<float, 5, 2> cY, cx, cy;
        Eigen::Matrix<float, 3, 4> Mx, My;

        void init();

    };

}

