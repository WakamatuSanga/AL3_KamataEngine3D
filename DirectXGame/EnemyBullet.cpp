#include "EnemyBullet.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	model_ = model;
	velocity_ = velocity;
	lifeTimer_ = 0;
	isDead_ = false;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.3f, 0.3f, 0.9f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = position;
}

void EnemyBullet::FaceToVelocity_() {
	const float vx = velocity_.x, vy = velocity_.y, vz = velocity_.z;
	const float lenXZ = std::sqrt(vx * vx + vz * vz);
	const float yaw = std::atan2(vx, vz);       // +Z が前
	const float pitch = std::atan2(-vy, lenXZ); // 上下
	worldTransform_.rotation_ = {pitch, yaw, 0.0f};
}

void EnemyBullet::KillIfOutOfRange_() {
	// シンプルな広域カリング（ステージに合わせて調整）
	const auto& p = worldTransform_.translation_;
	if (p.z < -60.0f || p.z > 200.0f || std::fabs(p.x) > 100.0f || std::fabs(p.y) > 100.0f) {
		isDead_ = true;
	}
}

float EnemyBullet::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.5f;
	return kBase * m;
}

void EnemyBullet::Update() {
	if (isDead_)
		return;

	// 位置更新
	worldTransform_.translation_ += velocity_;

	// 進行方向に向ける（必要なときだけ）
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

	// 画面外カリング
	KillIfOutOfRange_();
}

void EnemyBullet::Draw(Camera& camera) {
	if (isDead_ || !model_)
		return;
	model_->Draw(worldTransform_, camera);
}
