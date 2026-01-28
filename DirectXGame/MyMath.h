#pragma once
#include "KamataEngine.h"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace KamataEngine;

// 円周率
const float PI = 3.141592654f;

struct AABB {
	Vector3 min;
	Vector3 max;
};

// -----------------------------------------------------
// 演算子オーバーロードの宣言
// -----------------------------------------------------

// 単項演算子 (+v, -v)
Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v);

// 2項演算子 (v1 + v2, v1 - v2, v * float)
const Vector3 operator+(const Vector3& v1, const Vector3& v2);
const Vector3 operator-(const Vector3& v1, const Vector3& v2);
const Vector3 operator*(const Vector3& v1, float s);
const Vector3 operator*(float s, const Vector3& v1);
const Vector3 operator/(const Vector3& v1, float s);

// 複合代入演算子 (+=, -=, *=, /=)
Vector3& operator+=(Vector3& lhs, const Vector3& rhv);
Vector3& operator-=(Vector3& lhs, const Vector3& rhv);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);

// -----------------------------------------------------
// 便利な関数群
// -----------------------------------------------------

float Dot(const Vector3& a, const Vector3& b);
float Length(const Vector3& v);
Vector3 Normalized(const Vector3& v);

// 球面線形補間 (Slerp)
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);
// 線形補間 (Lerp)
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
float Lerp(float x1, float x2, float t);

// イージング
float EaseIn(float x1, float x2, float t);
float EaseOut(float x1, float x2, float t);
float EaseInOut(float x1, float x2, float t);

// ベクトル変換 (平行移動あり/なし)
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

// 行列作成
Matrix4x4 MakeIdentityMatrix();
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
Matrix4x4 MakeRotateXMatrix(float theta);
Matrix4x4 MakeRotateYMatrix(float theta);
Matrix4x4 MakeRotateZMatrix(float theta);
Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

// ★追加：透視投影行列の作成
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearZ, float farZ);
// ★追加：ビューポート行列の作成
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

// アフィン変換行列の作成
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);

Matrix4x4 Inverse(const Matrix4x4& m);

// 行列演算
Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

// ワールドトランスフォーム更新
void WorldTransformUpdate(WorldTransform& worldTransform);

// 衝突判定
bool IsCollision(const AABB& aabb1, const AABB& aabb2);

// スプライン
Vector3 CatmullRom(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);
Vector3 CatmullRomSpline(const std::vector<Vector3>& controlPoints, float t);

// 補助
inline float ToRadians(float degrees) { return degrees * (3.1415f / 180.0f); }
inline float ToDegrees(float radians) { return radians * (180.0f / 3.1415f); }