#include "PlayerBullet.h"
#include "MyMath.h"
#include <cmath>
#include <algorithm>

using namespace KamataEngine;

void PlayerBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	model_ = model;
	velocity_ = velocity;
	lifeTimer_ = 0;
	isDead_ = false;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.3f, 0.3f, 0.3f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = position;
}

void PlayerBullet::FaceToVelocity_() {
	const float vx = velocity_.x, vy = velocity_.y, vz = velocity_.z;
	const float lenXZ = std::sqrt(vx * vx + vz * vz);
	// +Z が前提のヨー/ピッチ
	const float yaw = std::atan2(vx, vz);
	const float pitch = std::atan2(-vy, lenXZ);
	worldTransform_.rotation_ = {pitch, yaw, 0.0f};
}
float PlayerBullet::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.5f; // 素モデルの半径（単位球/キューブ相当の目安）
	return kBase * m;
}
void PlayerBullet::Update() {
	if (isDead_)
		return;

	// 位置更新
	worldTransform_.translation_ += velocity_;

	// 見た目を進行方向へ向ける（必要なときだけ）
	if (alignToVelocity_) {
		FaceToVelocity_();
	}

	// 行列計算＆転送
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 寿命
	++lifeTimer_;
	if (lifeTimer_ >= lifeTime_) {
		isDead_ = true;
	}
}

void PlayerBullet::Draw(Camera& camera) {
	if (isDead_ || !model_)
		return;
	model_->Draw(worldTransform_, camera);
}
