#include "RailCamera.h"
#include "MyMath.h"
#include <cmath>
using namespace KamataEngine;

void RailCamera::Initialize(Spline* sp, Camera* cam) {
	sp_ = sp;
	cam_ = cam;
	up_ = {0, 1, 0};
	prevFwd_ = {0, 0, 1};
}

// 左手系ビュー行列（D3DのLookAtLH相当：前方 +Z）
Matrix4x4 RailCamera::BuildView(const Vector3& eye, const Vector3& target, const Vector3& upWorld) {
	Vector3 f = Normalized(Vector3{target.x - eye.x, target.y - eye.y, target.z - eye.z}); // +Z 前
	Vector3 upFix = upWorld;
	float d = std::fabs(f.x * upWorld.x + f.y * upWorld.y + f.z * upWorld.z);
	if (d > 0.999f)
		upFix = {0, 1, 0};

	// ★ 左手系：xaxis = normalize(cross(Up, f))
	Vector3 r = Normalized(Cross(upFix, f));
	Vector3 u = Cross(f, r);

	Matrix4x4 m{}; // 行優先
	m.m[0][0] = r.x;
	m.m[0][1] = u.x;
	m.m[0][2] = f.x;
	m.m[0][3] = 0.0f;
	m.m[1][0] = r.y;
	m.m[1][1] = u.y;
	m.m[1][2] = f.y;
	m.m[1][3] = 0.0f;
	m.m[2][0] = r.z;
	m.m[2][1] = u.z;
	m.m[2][2] = f.z;
	m.m[2][3] = 0.0f;
	m.m[3][0] = -Dot(r, eye);
	m.m[3][1] = -Dot(u, eye);
	m.m[3][2] = -Dot(f, eye);
	m.m[3][3] = 1.0f;
	return m;
}


// ★ FOV から可視半幅/半高を得る
void RailCamera::GetViewPlaneHalfExtents(float dist, float& halfW, float& halfH) const {
	// 一般的な透視投影：m00 = 1/tan(fovX/2), m11 = 1/tan(fovY/2)
	float m00 = cam_->matProjection.m[0][0];
	float m11 = cam_->matProjection.m[1][1];
	if (m00 == 0.0f || m11 == 0.0f) {
		// フォールバック（縦60°、16:9想定）
		float fovY = 60.0f * 3.14159265f / 180.0f;
		float tanY = std::tan(fovY * 0.5f);
		float aspect = 16.0f / 9.0f;
		halfH = dist * tanY;
		halfW = halfH * aspect;
	} else {
		halfW = dist / m00;
		halfH = dist / m11;
	}
}

void RailCamera::Update() {
	if (!sp_ || !cam_)
		return;

	t_ += speedPerFrame_;
	pos_ = sp_->Pos(t_);
	fwd_ = sp_->Tan(t_);

	// ★ 左手系：right = normalize(cross(up, fwd))
	right_ = Normalized(Cross(up_, fwd_));
	// ★ 左手系：up = cross(fwd, right) で再直交化
	up_ = Normalized(Cross(fwd_, right_));

	// バンクはそのまま
	Vector3 df = {fwd_.x - prevFwd_.x, fwd_.y - prevFwd_.y, fwd_.z - prevFwd_.z};
	float curv = Length(df);
	float bank = std::fmax(-bankMax_, std::fmin(bankMax_, curv * bankK_));
	RotateAroundForward(bank, right_, up_);
	prevFwd_ = fwd_;

	// 位置と注視（+Z 前提）
	eye_ = pos_ + (-(fwd_ * followDist_));
	Vector3 target = pos_ + (fwd_ * lookAhead_);

	cam_->matView = BuildView(eye_, target, up_);
	cam_->TransferMatrix();
}
