/**
 * Copyright (c) 2014-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant 
 * of patent rights can be found in the PATENTS file in the same directory.
 */
#ifndef FBX2GLTF_MATHFU_H
#define FBX2GLTF_MATHFU_H

#include <fbxsdk.h>

#include <mathfu/vector.h>
#include <mathfu/matrix.h>
#include <mathfu/quaternion.h>
#include <mathfu/rect.h>

/**
 * All the mathfu:: implementations of our core data types.
 */

template<class T, int d>
struct Bounds
{
    mathfu::Vector<T, d> min;
    mathfu::Vector<T, d> max;
    bool initialized = false;

    void Clear() {
        min = mathfu::Vector<T, d>();
        max = mathfu::Vector<T, d>();
        initialized = false;
    }

    void AddPoint(const mathfu::Vector<T, d> &p) {
        if (initialized) {
            for (int ii = 0; ii < d; ii ++) {
                min(ii) = std::min(min(ii), p(ii));
                max(ii) = std::max(max(ii), p(ii));
            }
        } else {
            min = p;
            max = p;
            initialized = true;
        }
    }
};

typedef mathfu::Vector<uint16_t, 4> Vec4i;
typedef mathfu::Matrix<uint16_t, 4> Mat4i;
typedef mathfu::Vector<float, 2>    Vec2f;
typedef mathfu::Vector<float, 3>    Vec3f;
typedef mathfu::Vector<float, 4>    Vec4f;
typedef mathfu::Matrix<float, 2>    Mat2f;
typedef mathfu::Matrix<float, 3>    Mat3f;
typedef mathfu::Matrix<float, 4>    Mat4f;
typedef mathfu::Quaternion<float>   Quatf;
typedef Bounds<float, 3>            Boundsf;

template<class T, int d> static inline std::vector<T> toStdVec(const mathfu::Vector <T, d> &vec)
{
    std::vector<T> result(d);
    for (int ii = 0; ii < d; ii ++) {
        result[ii] = vec[ii];
    }
    return result;
}

template<class T> std::vector<T> toStdVec(const mathfu::Quaternion<T> &quat) {
    return std::vector<T> { quat.vector()[0], quat.vector()[1], quat.vector()[2], quat.scalar() };
}

static inline Vec3f toVec3f(const FbxVector4 &v) {
    return Vec3f((float) v[0], (float) v[1], (float) v[2]);
}

static inline Mat4f toMat4f(const FbxAMatrix &m) {
    auto result = Mat4f();
    for (int row = 0; row < 4; row ++) {
        for (int col = 0; col < 4; col ++) {
            result(row, col) = (float) m[row][col];
        }
    }
    return result;
}

static inline Quatf toQuatf(const FbxQuaternion &q) {
    return Quatf((float) q[3], (float) q[0], (float) q[1], (float) q[2]);
}

#endif //FBX2GLTF_MATHFU_H
