#pragma once
#include "KamataEngine.h"
#include "Spline.h"
using namespace KamataEngine;

class RailCamera {
public:
	void Initialize(Spline* sp, Camera* cam);
	void Update();

	// ★ カメラの前 dist にある平面の中心
	Vector3 ScreenPlaneCenter(float dist) const { return eye_ + (fwd_ * dist); }

	// ★ 平面距離 dist における「半分の見える幅/高さ」を返す（FOVから計算）
	void GetViewPlaneHalfExtents(float dist, float& halfW, float& halfH) const;

	const Vector3& Fwd() const { return fwd_; }
	const Vector3& Right() const { return right_; }
	const Vector3& Up() const { return up_; }
	const Vector3& Eye() const { return eye_; }

	void SetSpeedPerFrame(float v) { speedPerFrame_ = v; }
	void SetFollowDist(float v) { followDist_ = v; }
	void SetLookAhead(float v) { lookAhead_ = v; }
	void SetBankParam(float k, float maxRad) {
		bankK_ = k;
		bankMax_ = maxRad;
	}

private:
	static Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
	static float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
	static void RotateAroundForward(float rad, Vector3& right, Vector3& up) {
		float c = std::cos(rad), s = std::sin(rad);
		Vector3 r = {right.x * c + up.x * s, right.y * c + up.y * s, right.z * c + up.z * s};
		Vector3 u = {up.x * c - right.x * s, up.y * c - right.y * s, up.z * c - right.z * s};
		right = r;
		up = u;
	}
	static Matrix4x4 BuildView(const Vector3& eye, const Vector3& target, const Vector3& upWorld);

private:
	Spline* sp_ = nullptr;
	Camera* cam_ = nullptr;
	float t_ = 0.0f;
	float speedPerFrame_ = 0.02f;
	float followDist_ = 6.0f;
	float lookAhead_ = 3.5f;
	float bankK_ = 0.85f, bankMax_ = 0.6f;

	Vector3 pos_{}, fwd_{0, 0, 1}, up_{0, 1, 0}, prevFwd_{0, 0, 1};
	Vector3 eye_{}, right_{1, 0, 0};
};
