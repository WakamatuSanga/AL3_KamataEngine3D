#include "EnemyAimer.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

EnemyAimer::~EnemyAimer() {
	for (auto* b : bullets_) {
		delete b;
	}
	bullets_.clear();
	delete bulletModel_; // エンジンの共有管理なら不要
}

void EnemyAimer::Initialize(Model* model, Player* player) {
	model_ = model;
	player_ = player;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {5.0f, 0.0f, 25.0f}; // 右寄り・奥から

	bulletModel_ = Model::CreateFromOBJ("enemyBullet");
	shotTimer_ = 0;
}

void EnemyAimer::FireAimedBullet() {
	if (!bulletModel_ || !player_)
		return;

	const float kBulletSpeed = 0.6f;

	Vector3 enemyPos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 dir = playerPos;
	dir -= enemyPos;
	dir = Normalized(dir);

	Vector3 vel = dir * kBulletSpeed;

	auto* b = new EnemyBullet();
	b->Initialize(bulletModel_, enemyPos, vel);

	// 自機狙い弾だけ進行方向に向ける
	b->SetAlignToVelocity(true);

	bullets_.push_back(b);
}

void EnemyAimer::RespawnIfFar() {
	const auto& p = worldTransform_.translation_;
	if (p.z < -40.0f || std::fabs(p.x) > 40.0f || std::fabs(p.y) > 40.0f) {
		for (auto* b : bullets_) {
			delete b;
		}
		bullets_.clear();

		worldTransform_.translation_ = {5.0f, 0.0f, 25.0f};
		shotTimer_ = 0;
	}
}

float EnemyAimer::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f;
	return kBase * m;
}

void EnemyAimer::Update() {
	// 手前へ移動
	worldTransform_.translation_.z += moveSpeedZ_;

	// プレイヤーを通り過ぎるまで発射
	if (player_) {
		const float playerZ = player_->GetPosition().z;
		const bool hasPassed = (worldTransform_.translation_.z <= playerZ + stopShootMarginZ_);

		if (!hasPassed) {
			++shotTimer_;
			if (shotTimer_ >= shotInterval_) {
				shotTimer_ = 0;
				FireAimedBullet();
			}
		}
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 弾更新＆掃除
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

	// 範囲外ならリスポーン
	RespawnIfFar();
}

void EnemyAimer::Draw(Camera& camera) {
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
	for (EnemyBullet* b : bullets_) {
		b->Draw(camera);
	}
}
