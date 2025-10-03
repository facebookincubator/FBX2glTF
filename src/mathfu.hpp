/**
 * Replacement of mathfu.hpp using GLM (https://github.com/g-truc/glm)
 *
 * Install: brew install glm  (mac)  或者 add to vcpkg / submodule
 *
 * Notes:
 * - GLM is header-only. Add include path if installed via package manager.
 * - This preserves the typedefs (Vec3f, Mat4f, Quatf, etc.) used in the codebase.
 */

#pragma once

#include <array>
#include <vector>
#include <algorithm>

#include <fbxsdk.h>

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

template <class T, int d>
struct Bounds {
  // use std::vector<T> for storage or keep explicit math types via glm
  std::array<T, d> min;
  std::array<T, d> max;
  bool initialized = false;

  void Clear() {
    for (int i = 0; i < d; ++i) {
      min[i] = T(0);
      max[i] = T(0);
    }
    initialized = false;
  }

  void AddPoint(const glm::tvec3<T>& p) {
    if (initialized) {
      for (int ii = 0; ii < d; ii++) {
        min[ii] = std::min(min[ii], p[ii]);
        max[ii] = std::max(max[ii], p[ii]);
      }
    } else {
      for (int ii = 0; ii < d; ii++) {
        min[ii] = p[ii];
        max[ii] = p[ii];
      }
      initialized = true;
    }
  }
};

// typedefs mapping to GLM types
typedef glm::u16vec4 Vec4i;
typedef glm::mat<4, 4, uint16_t, glm::defaultp> Mat4i;
typedef glm::vec2 Vec2f;
typedef glm::vec3 Vec3f;
typedef glm::vec4 Vec4f;
typedef glm::mat<2, 2, float, glm::defaultp> Mat2f;
typedef glm::mat<3, 3, float, glm::defaultp> Mat3f;
typedef glm::mat<4, 4, float, glm::defaultp> Mat4f;
typedef glm::quat Quatf;
typedef Bounds<float, 3> Boundsf;

#define VEC3F_ONE (Vec3f{1.0f, 1.0f, 1.0f})
#define VEC3F_ZERO (Vec3f{0.0f, 0.0f, 0.0f})
#define VEC4F_ONE (Vec4f{1.0f, 1.0f, 1.0f, 1.0f})
#define VEC4F_ZERO (Vec4f{0.0f, 0.0f, 0.0f, 0.0f})

template <class T, int d>
inline std::vector<T> toStdVec(const glm::tvec3<T>& vec) {
  std::vector<T> result(d);
  for (int ii = 0; ii < d; ii++) {
    result[ii] = vec[ii];
  }
  return result;
}

template <class T>
std::vector<T> toStdVec(const glm::qua<T>& quat) {
  return std::vector<T>{quat.x, quat.y, quat.z, quat.w};
}

// added: explicit overloads for project typedefs (Vec2f/Vec3f/Vec4f/Quatf)
inline std::vector<float> toStdVec(const Vec2f& v) {
  return std::vector<float>{v.x, v.y};
}
inline std::vector<float> toStdVec(const Vec3f& v) {
  return std::vector<float>{v.x, v.y, v.z};
}
inline std::vector<float> toStdVec(const Vec4f& v) {
  return std::vector<float>{v.x, v.y, v.z, v.w};
}
inline std::vector<float> toStdVec(const Quatf& q) {
  return std::vector<float>{q.x, q.y, q.z, q.w};
}

// added: support std::array<T, d> (used by Bounds::min/max)
template <class T, size_t d>
inline std::vector<T> toStdVec(const std::array<T, d>& arr) {
  return std::vector<T>(arr.begin(), arr.end());
}

inline Vec3f toVec3f(const FbxDouble3& v) {
  return Vec3f((float)v[0], (float)v[1], (float)v[2]);
}

inline Vec3f toVec3f(const FbxVector4& v) {
  return Vec3f((float)v[0], (float)v[1], (float)v[2]);
}

inline Vec4f toVec4f(const FbxVector4& v) {
  return Vec4f((float)v[0], (float)v[1], (float)v[2], (float)v[3]);
}

inline Mat4f toMat4f(const FbxAMatrix& m) {
  Mat4f result(1.0f);
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      result[col][row] = (float)m[row][col]; // glm is column-major
    }
  }
  return result;
}

inline Quatf toQuatf(const FbxQuaternion& q) {
  // FbxQuaternion is [x,y,z,w]? original code used (w,x,y,z)
  return Quatf((float)q[3], (float)q[0], (float)q[1], (float)q[2]);
}
