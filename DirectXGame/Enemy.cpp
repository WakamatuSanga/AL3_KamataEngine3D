#include "Enemy.h"
#include "MyMath.h"
#include <cmath>
#include <algorithm>
using namespace KamataEngine;

Enemy::~Enemy() {
	for (auto* b : bullets_) {
		delete b;
	}
	bullets_.clear();
	delete bulletModel_;
}

void Enemy::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 25.0f};

	approachVelocity_ = {0.0f, 0.0f, -0.2f};
	leaveVelocity_ = {0.3f, 0.2f, -0.1f};
	phase_ = Phase::Approach;

	// ★ 弾モデル（テクスチャ無しのシンプルなやつ）
	bulletModel_ = Model::CreateFromOBJ("enemyBullet");
}

// 弾を1発撃つ
void Enemy::FireBullet() {
	if (!bulletModel_)
		return;

	const float kBulletSpeed = 0.5f;
	// 今は Z マイナス方向（カメラ側）に撃つ
	Vector3 vel{0.0f, 0.0f, -kBulletSpeed};

	EnemyBullet* b = new EnemyBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, vel);
	bullets_.push_back(b);
}

float Enemy::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f; // 見た目に合わせて
	return kBase * m;
}

void Enemy::UpdateApproach() {
	worldTransform_.translation_ += approachVelocity_;

	// 一定位置まで来たら離脱へ
	if (worldTransform_.translation_.z < 0.0f) {
		phase_ = Phase::Leave;
	}

	// ★ 一定間隔で発射
	++shotTimer_;
	if (shotTimer_ >= shotInterval_) {
		shotTimer_ = 0;
		FireBullet();
	}
}

void Enemy::UpdateLeave() {
	worldTransform_.translation_ += leaveVelocity_;

	// ★ std::fabsf に
	if (worldTransform_.translation_.z < -40.0f || std::fabsf(worldTransform_.translation_.x) > 40.0f || std::fabsf(worldTransform_.translation_.y) > 40.0f) {
		Respawn();
	}
}
void Enemy::Respawn() {
	// 弾を全部消す
	for (auto* b : bullets_) {
		delete b;
	}
	bullets_.clear();

	// ★ 出現位置（少しランダム化したい場合）
	//    例：X[-6,+6], Y[-3,+3], Z=+25
	auto frand = [](float a, float b) { return a + (b - a) * (float)rand() / (float)RAND_MAX; };
	worldTransform_.translation_ = {frand(-6.0f, 6.0f), frand(-3.0f, 3.0f), 25.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	// ★ 透明度を使っていた場合のリセット（使っていなければ無視されるだけ）
	if (model_) {
		model_->SetAlpha(1.0f);
	}

	// フェーズとタイマーをリセット
	phase_ = Phase::Approach;
	shotTimer_ = 0;
}

void Enemy::Update() {
	switch (phase_) {
	case Phase::Approach:
	default:
		UpdateApproach();
		break;
	case Phase::Leave:
		UpdateLeave();
		break;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// ★ 弾の更新＆削除
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		EnemyBullet* b = *it;
		b->Update();
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
}

void Enemy::Draw(Camera& camera) {
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}

	// ★ 弾の描画
	for (EnemyBullet* b : bullets_) {
		b->Draw(camera);
	}
}
