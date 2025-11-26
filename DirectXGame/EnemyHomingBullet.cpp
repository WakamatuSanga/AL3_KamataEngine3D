#include "EnemyHomingBullet.h"
#include <cmath>
#include <algorithm>

using namespace KamataEngine;

void EnemyHomingBullet::Initialize(Model* model, const Vector3& spawnPos, float speed, float turnRate, const Player* player) {
	model_ = model;
	player_ = player;
	speed_ = speed;
	turn_ = std::clamp(turnRate, 0.0f, 1.0f);
	life_ = 360;
	isDead_ = false;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {0.25f, 0.25f, 0.50f};
	worldTransform_.rotation_ = {0, 0, 0};
	worldTransform_.translation_ = spawnPos;

	// 初速は「奥へ真っ直ぐ」(+Z) から開始
	velocity_ = {0, 0, speed_};

	
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void EnemyHomingBullet::FaceToVelocity_() {
	Vector3 v = velocity_;
	float lenXZ = std::sqrt(v.x * v.x + v.z * v.z);
	float yaw = std::atan2(v.x, v.z);      // +Z が前
	float pitch = std::atan2(-v.y, lenXZ); // 上下
	worldTransform_.rotation_ = {pitch, yaw, 0.0f};
}

float EnemyHomingBullet::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.5f;
	return kBase * m;
}

void EnemyHomingBullet::Update() {
	if (isDead_)
		return;

	// --- 誘導（距離に応じて弱め→一定距離で固定） ---
	if (homingActive_ && player_) {
		Vector3 p = player_->GetPosition();
		Vector3 toP = {p.x - worldTransform_.translation_.x, p.y - worldTransform_.translation_.y, p.z - worldTransform_.translation_.z};

		float dist = Length(toP);

		// ★ 距離が lockDist_ 以下になったら“その時点の向きのまま”固定直進
		if (dist <= lockDist_) {
			homingActive_ = false;
			velocity_ = Normalized(velocity_) * speed_; // 向きだけ固定して等速
		} else {
			// ★ フェード範囲： [fadeStartDist_ .. lockDist_] の間で turn を 1→0 にスムーズ減衰
			float t = turn_;
			if (dist < fadeStartDist_) {
				float u = (dist - lockDist_) / max(1e-3f, (fadeStartDist_ - lockDist_));
				u = std::clamp(u, 0.0f, 1.0f); // lockDist_で0, fadeStartDist_で1
				t *= u;
			}

			// Slerp で“少しだけ”向きを合わせる
			Vector3 want = Normalized(toP) * speed_;
			velocity_ = Slerp(velocity_, want, t);
		}
	}

	// --- 前進 ---
	worldTransform_.translation_ += velocity_;

	// 見た目を進行方向へ
	FaceToVelocity_();

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 寿命
	if (--life_ <= 0)
		isDead_ = true;

	// 簡易カリング
	const auto& pos = worldTransform_.translation_;
	if (pos.z < -60.0f || pos.z > 200.0f || std::fabs(pos.x) > 120.0f || std::fabs(pos.y) > 120.0f) {
		isDead_ = true;
	}
}

void EnemyHomingBullet::Draw(Camera& cam) {
	if (isDead_ || !model_)
		return;
	model_->Draw(worldTransform_, cam);
}
