/*
	Cornell CS5625
    Microfacet model reference implementation

    modified from Nori educational raytracer
    Copyright (c) 2012 by Wenzel Jakob and Steve Marschner.

	DO NOT REDISTRIBUTE
*/

#include <Eigen/Core>

#include "microfacet.hpp"
#include "frame.hpp"
#include "geomtools.hpp"

namespace nori {

	/* A few useful constants */
	#define INV_PI       0.31830988618379067154f
	#define INV_TWOPI    0.15915494309189533577f
	#define INV_FOURPI   0.07957747154594766788f
	#define SQRT_TWO     1.41421356237309504880f
	#define INV_SQRT_TWO 0.70710678118654752440f


	float fresnel(float cosThetaI, float extIOR, float intIOR) {
		float etaI = extIOR, etaT = intIOR;

		if (extIOR == intIOR)
			return 0.0f;

		/* Swap the indices of refraction if the interaction starts
		   at the inside of the object */
		if (cosThetaI < 0.0f) {
			std::swap(etaI, etaT);
			cosThetaI = -cosThetaI;
		}

		/* Using Snell's law, calculate the squared sine of the
		   angle between the normal and the transmitted ray */
		float eta = etaI / etaT,
			  sinThetaTSqr = eta*eta * (1-cosThetaI*cosThetaI);

		if (sinThetaTSqr > 1.0f)
			return 1.0f;  /* Total internal reflection! */

		float cosThetaT = std::sqrt(1.0f - sinThetaTSqr);

		float Rs = (etaI * cosThetaI - etaT * cosThetaT)
		         / (etaI * cosThetaI + etaT * cosThetaT);
		float Rp = (etaT * cosThetaI - etaI * cosThetaT)
		         / (etaT * cosThetaI + etaI * cosThetaT);

		return (Rs * Rs + Rp * Rp) / 2.0f;
	}

	/// Evaluate the microfacet normal distribution D
	float Microfacet::evalBeckmann(const Normal3f &m) const {
		float temp = Frame::tanTheta(m) / m_alpha,
			  ct = Frame::cosTheta(m), ct2 = ct*ct;

		return std::exp(-temp*temp) 
			/ (M_PI * m_alpha * m_alpha * ct2 * ct2);
	}

	/// Sample D*cos(thetaM)
	Normal3f Microfacet::sampleBeckmann(const Point2f &sample) const {
		float sinPhi, cosPhi;
		math::sincos(2.0f * M_PI * sample.x(), &sinPhi, &cosPhi);

		float tanThetaMSqr = -m_alpha*m_alpha * std::log(1.0f - sample.y());
		float cosThetaM = 1.0f / std::sqrt(1+tanThetaMSqr);
		float sinThetaM = std::sqrt(std::max(0.0f, 1.0f - cosThetaM*cosThetaM));

		return Normal3f(sinThetaM * cosPhi, sinThetaM * sinPhi, cosThetaM);
	}

	/// Evaluate Smith's shadowing-masking function G1 
	float Microfacet::smithBeckmannG1(const Vector3f &v, const Normal3f &m) const {
		float tanTheta = Frame::tanTheta(v);

		/* Perpendicular incidence -- no shadowing/masking */
		if (tanTheta == 0.0f)
			return 1.0f;

		/* Can't see the back side from the front and vice versa */
		if (m.dot(v) * Frame::cosTheta(v) <= 0)
			return 0.0f;

		float a = 1.0f / (m_alpha * tanTheta);
		if (a >= 1.6f)
			return 1.0f;
		float a2 = a * a;

		/* Use a fast and accurate (<0.35% rel. error) rational
		   approximation to the shadowing-masking function */
		return (3.535f * a + 2.181f * a2) 
		     / (1.0f + 2.276f * a + 2.577f * a2);
	}

	/// Evaluate the BRDF for the given pair of directions
	Color3f Microfacet::eval(const BSDFQueryRecord &bRec) const {
		/* Return zero queried for illumination on the backside */
		if (Frame::cosTheta(bRec.wi) <= 0
			|| Frame::cosTheta(bRec.wo) <= 0)
			return Color3f(0, 0, 0);

		/* Compute the half direction vector */
		Normal3f H = (bRec.wo+bRec.wi).normalized();

		/* Evaluate the microsurface normal distribution */
		float D = evalBeckmann(H);

		/* Fresnel factor */
		float F = fresnel(H.dot(bRec.wi), m_extIOR, m_intIOR);

		/* Smith's shadow-masking function */
		float G = smithBeckmannG1(bRec.wi, H) * 
		          smithBeckmannG1(bRec.wo, H);

		/* Combine everything to obtain the specular reflectance */
		float spec = m_ks * F * D * G / 
			(4.0f * Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo));

		return m_R * INV_PI + Color3f(spec, spec, spec);
	}

	/// Evaluate the sampling density of \ref sample() wrt. solid angles
	float Microfacet::pdf(const BSDFQueryRecord &bRec) const {
		/* Return zero when queried for illumination on the backside */
		if (Frame::cosTheta(bRec.wi) <= 0
			|| Frame::cosTheta(bRec.wo) <= 0)
			return 0.0f;

		/* Compute the half direction vector */
		Normal3f H = (bRec.wo+bRec.wi).normalized();

		/* Microfacet sampling density */
		float D_density = evalBeckmann(H) * Frame::cosTheta(H);

		/* Jacobian of the half-direction mapping */
		float dwh_dwo = 1.0f / (4.0f * H.dot(bRec.wo));
			
		return (D_density * dwh_dwo) * m_ks + 
			(INV_PI * Frame::cosTheta(bRec.wo)) * (1-m_ks);
	}

	/// Sample the BRDF
	Color3f Microfacet::sample(BSDFQueryRecord &bRec, const Point2f &_sample) const {
		Point2f sample(_sample);

		if (sample.x() < m_ks) {
			/* Sample the specular component */
			sample.x() /= m_ks;

			/* Sample a microsurface normal */
			Normal3f m = sampleBeckmann(sample);
			
			/* Compute the exitant direction */
			bRec.wo = 2 * m.dot(bRec.wi) * m - bRec.wi;

		} else {
			/* Sample the diffuse component */
			sample.x() = (sample.x() - m_ks) / (1-m_ks);
			bRec.wo = RTUtil::squareToCosineHemisphere(sample);
		}
		
		/* Relative index of refraction: no change */
		bRec.eta = 1.0f;

		float pdfval = pdf(bRec);
		if (pdfval == 0) {
			/* Reject samples outside of the positive 
			   hemisphere, which have zero probability */
			return Color3f(0, 0, 0);
		}
		
		return eval(bRec) * (Frame::cosTheta(bRec.wo) / pdfval);
	}

	Color3f Microfacet::diffuseReflectance() const {
		return m_R;
	}

}
