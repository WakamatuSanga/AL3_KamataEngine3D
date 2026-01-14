#include "MyMath.h"
#include <cmath>
#include <numbers>

using namespace KamataEngine;

// =================================================================
// Vector3 演算子オーバーロードの実装
// =================================================================

// 単項 +
Vector3 operator+(const Vector3& v) { return v; }
// 単項 -
Vector3 operator-(const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); }

// 2項 +
const Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	return temp += v2;
}

// 2項 -
const Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	return temp -= v2;
}

// 2項 * (Vector3 * float)
const Vector3 operator*(const Vector3& v1, float s) {
	Vector3 temp(v1);
	return temp *= s;
}
// 2項 * (float * Vector3)
const Vector3 operator*(float s, const Vector3& v1) { return v1 * s; }
// 2項 /
const Vector3 operator/(const Vector3& v1, float s) {
	Vector3 temp(v1);
	return temp /= s;
}

// 複合代入 +=
Vector3& operator+=(Vector3& lhv, const Vector3& rhv) {
	lhv.x += rhv.x;
	lhv.y += rhv.y;
	lhv.z += rhv.z;
	return lhv;
}

// 複合代入 -=
Vector3& operator-=(Vector3& lhv, const Vector3& rhv) {
	lhv.x -= rhv.x;
	lhv.y -= rhv.y;
	lhv.z -= rhv.z;
	return lhv;
}

// 複合代入 *=
Vector3& operator*=(Vector3& v, float s) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

// 複合代入 /=
Vector3& operator/=(Vector3& v, float s) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

// =================================================================
// ベクトル関数
// =================================================================

float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

float Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector3 Normalized(const Vector3& v) {
	float len = Length(v);
	if (len == 0.0f)
		return {0.0f, 0.0f, 0.0f};
	return v / len;
}

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) { return v1 + (v2 - v1) * t; }

Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t) {
	Vector3 n1 = Normalized(v1);
	Vector3 n2 = Normalized(v2);
	float d = Dot(n1, n2);

	// クランプ
	if (d > 1.0f)
		d = 1.0f;
	if (d < -1.0f)
		d = -1.0f;

	if (d > 0.9995f) {
		return Lerp(v1, v2, t);
	}

	float theta = std::acos(d);
	float sinTheta = std::sin(theta);
	if (std::fabs(sinTheta) < 1e-5f) {
		return Lerp(v1, v2, t);
	}

	float w1 = std::sin((1.0f - t) * theta) / sinTheta;
	float w2 = std::sin(t * theta) / sinTheta;

	Vector3 dir = n1 * w1 + n2 * w2;
	return Normalized(dir) * Lerp(Length(v1), Length(v2), t);
}

// =================================================================
// 数値補間
// =================================================================

float Lerp(float x1, float x2, float t) { return (1.0f - t) * x1 + t * x2; }

float EaseIn(float x1, float x2, float t) {
	float easedT = t * t;
	return Lerp(x1, x2, easedT);
}

float EaseOut(float x1, float x2, float t) {
	float easedT = 1.0f - std::powf(1.0f - t, 3.0f);
	return Lerp(x1, x2, easedT);
}

float EaseInOut(float x1, float x2, float t) {
	float easedT = -(std::cosf(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;
	return Lerp(x1, x2, easedT);
}

// =================================================================
// 衝突判定
// =================================================================

bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}

// =================================================================
// 行列・座標変換
// =================================================================

Matrix4x4 MakeIdentityMatrix() {
	static const Matrix4x4 result{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 result{scale.x, 0.0f, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, 0.0f, scale.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

Matrix4x4 MakeRotateXMatrix(float theta) {
	float s = std::sin(theta);
	float c = std::cos(theta);
	Matrix4x4 result{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

Matrix4x4 MakeRotateYMatrix(float theta) {
	float s = std::sin(theta);
	float c = std::cos(theta);
	Matrix4x4 result{c, 0.0f, -s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

Matrix4x4 MakeRotateZMatrix(float theta) {
	float s = std::sin(theta);
	float c = std::cos(theta);
	Matrix4x4 result{c, s, 0.0f, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle) {
	float c = std::cos(angle);
	float s = std::sin(angle);
	float t = 1.0f - c;
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	Matrix4x4 result;
	result.m[0][0] = t * x * x + c;
	result.m[0][1] = t * x * y + s * z;
	result.m[0][2] = t * x * z - s * y;
	result.m[0][3] = 0.0f;
	result.m[1][0] = t * x * y - s * z;
	result.m[1][1] = t * y * y + c;
	result.m[1][2] = t * y * z + s * x;
	result.m[1][3] = 0.0f;
	result.m[2][0] = t * x * z + s * y;
	result.m[2][1] = t * y * z - s * x;
	result.m[2][2] = t * z * z + c;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, translate.x, translate.y, translate.z, 1.0f};
	return result;
}

// ★ ここに追加：MakeAffineMatrixの実装
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate) {
	Matrix4x4 matScale = MakeScaleMatrix(scale);
	Matrix4x4 matRotX = MakeRotateXMatrix(rot.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rot.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rot.z);
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;
	Matrix4x4 matTrans = MakeTranslateMatrix(translate);

	// スケール * 回転 * 平行移動 の順で合成
	return matScale * matRot * matTrans;
}

Matrix4x4 Inverse(const Matrix4x4& m) {
	float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2];
	float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2];
	float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2];
	float tx = m.m[3][0], ty = m.m[3][1], tz = m.m[3][2];

	float c00 = (a11 * a22 - a12 * a21);
	float c01 = -(a10 * a22 - a12 * a20);
	float c02 = (a10 * a21 - a11 * a20);
	float c10 = -(a01 * a22 - a02 * a21);
	float c11 = (a00 * a22 - a02 * a20);
	float c12 = -(a00 * a21 - a01 * a20);
	float c20 = (a01 * a12 - a02 * a11);
	float c21 = -(a00 * a12 - a02 * a10);
	float c22 = (a00 * a11 - a01 * a10);

	float det = a00 * c00 + a01 * c01 + a02 * c02;
	if (std::fabs(det) < 1e-8f)
		return MakeIdentityMatrix();
	float invDet = 1.0f / det;

	Matrix4x4 inv{};
	inv.m[0][0] = c00 * invDet;
	inv.m[0][1] = c10 * invDet;
	inv.m[0][2] = c20 * invDet;
	inv.m[1][0] = c01 * invDet;
	inv.m[1][1] = c11 * invDet;
	inv.m[1][2] = c21 * invDet;
	inv.m[2][0] = c02 * invDet;
	inv.m[2][1] = c12 * invDet;
	inv.m[2][2] = c22 * invDet;
	// 逆平行移動
	inv.m[3][0] = -(tx * inv.m[0][0] + ty * inv.m[1][0] + tz * inv.m[2][0]);
	inv.m[3][1] = -(tx * inv.m[0][1] + ty * inv.m[1][1] + tz * inv.m[2][1]);
	inv.m[3][2] = -(tx * inv.m[0][2] + ty * inv.m[1][2] + tz * inv.m[2][2]);
	inv.m[3][3] = 1.0f;
	return inv;
}

Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm) {
	Matrix4x4 result{};
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++) {
			for (size_t k = 0; k < 4; k++) {
				result.m[i][j] += lhm.m[i][k] * rhm.m[k][j];
			}
		}
	}
	lhm = result;
	return lhm;
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = m1;
	return result *= m2;
}

void WorldTransformUpdate(WorldTransform& worldTransform) {
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
	if (worldTransform.parent_) {
		worldTransform.matWorld_ *= worldTransform.parent_->matWorld_;
	}
	worldTransform.TransferMatrix();
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];
	if (w != 0.0f) {
		result /= w;
	}
	return result;
}

Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m) {
	Vector3 r;
	r.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	r.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	r.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
	return r;
}

Vector3 CatmullRom(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t) {
	float t2 = t * t;
	float t3 = t2 * t;

	auto cr = [&](float v0, float v1, float v2, float v3) { return 0.5f * (2.0f * v1 + (-v0 + v2) * t + (2.0f * v0 - 5.0f * v1 + 4.0f * v2 - v3) * t2 + (-v0 + 3.0f * v1 - 3.0f * v2 + v3) * t3); };

	return {cr(p0.x, p1.x, p2.x, p3.x), cr(p0.y, p1.y, p2.y, p3.y), cr(p0.z, p1.z, p2.z, p3.z)};
}

Vector3 CatmullRomSpline(const std::vector<Vector3>& cps, float t) {
	size_t n = cps.size();
	if (n == 0)
		return {0, 0, 0};
	if (n == 1)
		return cps[0];
	if (n == 2)
		return Lerp(cps[0], cps[1], t);
	if (n == 3)
		return CatmullRom(cps[0], cps[0], cps[1], cps[2], t);

	if (t <= 0.0f)
		return cps[1];
	if (t >= 1.0f)
		return cps[n - 2];

	float segF = t * float(n - 3);
	int seg = static_cast<int>(segF);
	float localT = segF - float(seg);

	return CatmullRom(cps[seg], cps[seg + 1], cps[seg + 2], cps[seg + 3], localT);
}