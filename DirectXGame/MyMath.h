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

// 02_14 29枚目 単項演算子オーバーロード
Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v);

// 02_06のCameraControllerのUpdate/Reset関数で必要
const Vector3 operator+(const Vector3& lhv, const Vector3& rhv);

// 追加宣言
float Dot(const Vector3& a, const Vector3& b);

// 方向は球面補間、長さは線形補間（弾のホーミングで使う）
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

// 平行移動を無視した変換（法線/速度ベクトル用）
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

// 02_06のスライド24枚目のLerp関数
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

// 02_06 スライド29枚目で追加
const Vector3 operator*(const Vector3& v1, const float f);

// 代入演算子オーバーロード
Vector3& operator+=(Vector3& lhs, const Vector3& rhv);
Vector3& operator-=(Vector3& lhs, const Vector3& rhv);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);

// 単位行列の作成
Matrix4x4 MakeIdentityMatrix();
// スケーリング行列の作成
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// 回転行列の作成
Matrix4x4 MakeRotateXMatrix(float theta);
Matrix4x4 MakeRotateYMatrix(float theta);
Matrix4x4 MakeRotateZMatrix(float theta);
// 平行移動行列の作成
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// アフィン変換行列の作成
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);
// アフィン（最後の行が [0,0,0,1]）の逆行列を返す
Matrix4x4 Inverse(const Matrix4x4& m);

// 代入演算子オーバーロード
Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);

// 2項演算子オーバーロード
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

// ワールドトランスフォーム更新(02_03の最後)
void WorldTransformUpdate(WorldTransform& worldTransform);

float Lerp(float x1, float x2, float t);

float EaseIn(float x1, float x2, float t);

float EaseOut(float x1, float x2, float t);

float EaseInOut(float x1, float x2, float t);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

// 02_15 で追加
inline float ToRadians(float degrees) { return degrees * (3.1415f / 180.0f); }
inline float ToDegrees(float radians) { return radians * (180.0f / 3.1415f); }

// ベクトルの長さを求める
float Length(const Vector3& v);

// ベクトルを正規化する（方向だけにする）
Vector3 Normalized(const Vector3& v);

// 4点と区間パラメータ t(0～1) から Catmull-Rom 補間
Vector3 CatmullRom(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);

// 制御点配列全体に対して 0～1 の t から位置を求める
Vector3 CatmullRomSpline(const std::vector<Vector3>& controlPoints, float t);

// 任意の軸まわりの回転行列を作成する関数
Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);