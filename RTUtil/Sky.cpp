
#include "Sky.hpp"


using namespace RTUtil;


void Sky::init() {
    cY << 
     0.1787, -1.4630,
    -0.3554,  0.4275,
    -0.0227,  5.3251,
     0.1206, -2.5771,
    -0.0670,  0.3703;
    cx <<
    -0.0193, -0.2592,
    -0.0665,  0.0008,
    -0.0004,  0.2125,
    -0.0641, -0.8989,
    -0.0033,  0.0452;
    cy <<
    -0.0167, -0.2608,
    -0.0950,  0.0092,
    -0.0079,  0.2102,
    -0.0441, -1.6537,
    -0.0109,  0.0529;
    Mx <<
     0.0017, -0.0037,  0.0021,  0.0000,
    -0.0290,  0.0638, -0.0320,  0.0039,
     0.1169, -0.2120,  0.0605,  0.2589;
    My <<
     0.0028, -0.0061,  0.0032,  0.0000,
    -0.0421,  0.0897, -0.0415,  0.0052,
     0.1535, -0.2676,  0.0667,  0.2669;
}


// Compute the zenith luminance for given sky parameters
static float Y_z(float theta, float T) {
    float chi = (4./9. - T/120) * (M_PI - 2 * theta);
    return (4.0453 * T - 4.9710) * tan(chi) - 0.2155 * T + 2.4192;
}

// Compute the zenith chromaticity x or y for given sky
// parameters, given the matrix that defines Preetham's
// polynomial approximation for that component.
static float __z(float theta, float T, const Eigen::Matrix<float, 3, 4> &M) {
    Eigen::Vector3f vT(T*T, T, 1);
    Eigen::Vector4f vth(theta*theta*theta, theta*theta, theta, 1);
    return vT.transpose() * M * vth;
}

void Sky::setUniforms(GLWrap::Program &prog) {

    // Compute the parameters A, ..., E to the Perez model.  There is
    // a separate set of parameters for each of the three color 
    // components Y, x, and y.
    Eigen::Matrix<float, 5, 1> pY, px, py;
    pY = cY * Eigen::Vector2f(turbidity, 1);
    px = cx * Eigen::Vector2f(turbidity, 1);
    py = cy * Eigen::Vector2f(turbidity, 1);

    // Compute the zenith (looking straight up) color, which is
    // used to set the correct overall scale for each color component.
    float Yz = Y_z(theta_sun, turbidity);
    float xz = __z(theta_sun, turbidity, Mx);
    float yz = __z(theta_sun, turbidity, My);

    // Pass these results to the fragment shader
    prog.uniform("A", Eigen::Vector3f(pY[0], px[0], py[0]));
    prog.uniform("B", Eigen::Vector3f(pY[1], px[1], py[1]));
    prog.uniform("C", Eigen::Vector3f(pY[2], px[2], py[2]));
    prog.uniform("D", Eigen::Vector3f(pY[3], px[3], py[3]));
    prog.uniform("E", Eigen::Vector3f(pY[4], px[4], py[4]));
    prog.uniform("zenith", Eigen::Vector3f(Yz, xz, yz));
    prog.uniform("thetaSun", theta_sun);
}