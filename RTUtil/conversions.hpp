/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Some handy type conversion utilities for working with Assimp in a
 * program where you want to use Eigen as the principal matrix/vector
 * library.
 *
 * Author: srm, Spring 2020
 */

#pragma once

#include <Eigen/Core>
#include <assimp/scene.h>
#include <assimp/anim.h>


namespace RTUtil {

    // Convert a 3x3 Assimp matrix to a 3x3 Eigen matrix
    template <class T>
    inline Eigen::Matrix<T, 3, 3> a2e(aiMatrix3x3t<T> m) {
        Eigen::Matrix<T, 3, 3> e;
        e.matrix().transpose() = Eigen::Map<Eigen::Matrix<T, 3, 3>>(&m.a1);
        return e;
    }

    // Convert a 4x4 Assimp matrix to an Eigen 3D affine transformation
    template <class T>
    inline Eigen::Transform<T, 3, Eigen::Affine> a2e(aiMatrix4x4t<T> m) {
        Eigen::Transform<T, 3, Eigen::Affine> a;
        a.matrix().transpose() = Eigen::Map<Eigen::Matrix<T, 4, 4>>(&m.a1);
        return a;
    }

    // Convert a 3-vector from Assimp to Eigen
    template <class T>
    inline Eigen::Matrix<T, 3, 1> a2e(aiVector3t<T> v) {
        return Eigen::Map<Eigen::Matrix<T, 3, 1>>(&v.x);
    }

    // Convert a quaternion from Assimp to Eigen
    template <class T>
    inline Eigen::Quaternion<T> a2e(aiQuaterniont<T> v) {
        return Eigen::Quaternion<T>(v.w, v.x, v.y, v.z);
    }

    // Convert a 2-vector from Assimp to Eigen
    template <class T>
    inline Eigen::Matrix<T, 2, 1> a2e(aiVector2t<T> v) {
        return Eigen::Map<Eigen::Matrix<T, 2, 1>>(&v.x);
    }

    // Convert an RGB color from Assimp to an Eigen 3-vector
    inline Eigen::Matrix<ai_real, 3, 1> a2e(aiColor3D c) {
        return Eigen::Map<Eigen::Matrix<ai_real, 3, 1>>(&c.r);
    }

    // Convert an RGBA color from Assimp to an Eigen 4-vector
    inline Eigen::Matrix<ai_real, 4, 1> a2e(aiColor4D c) {
        return Eigen::Map<Eigen::Matrix<ai_real, 4, 1>>(&c.r);
    }

}