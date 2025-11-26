#include "EnemyHoming.h"
#include "MyMath.h"
#include <cmath>
#include <algorithm>
using namespace KamataEngine;

EnemyHoming::~EnemyHoming() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
	delete bulletModel_; // 共有管理なら削除不要。プロジェクト方針に合わせて。
}

float EnemyHoming::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f;
	return kBase * m;
}

void EnemyHoming::Initialize(Model* model, Player* player) {
	model_ = model;
	player_ = player;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {-4.0f, 1.0f, 25.0f}; // 例：左上・奥から

	// 弾モデル（OBJが無ければプリミティブへフォールバック）
	bulletModel_ = Model::CreateFromOBJ("homingBullet");
	if (!bulletModel_)
		bulletModel_ = Model::Create();

	shotTimer_ = 0;
}

void EnemyHoming::FireHomingBullet_() {
	if (!bulletModel_ || !player_)
		return;

	auto* b = new EnemyHomingBullet();
	KamataEngine::Vector3 spawn = worldTransform_.translation_;
	b->Initialize(bulletModel_, spawn, bulletSpeed_, turnRate_, player_);
	bullets_.push_back(b);
}

void EnemyHoming::RespawnIfFar_() {
	const auto& p = worldTransform_.translation_;
	if (p.z < -40.0f || std::fabs(p.x) > 40.0f || std::fabs(p.y) > 40.0f) {
		for (auto* b : bullets_)
			delete b;
		bullets_.clear();
		// 位置リセット（軽くランダムでもOK）
		worldTransform_.translation_ = {-4.0f, 1.0f, 25.0f};
		shotTimer_ = 0;
	}
}

void EnemyHoming::Update() {
	// 手前へ移動
	worldTransform_.translation_.z += moveSpeedZ_;

	// 一定間隔で発射
	++shotTimer_;
	if (shotTimer_ >= shotInterval_) {
		shotTimer_ = 0;
		FireHomingBullet_();
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 弾更新＆掃除
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		auto* b = *it;
		b->Update();
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}

	// 範囲外で再出現
	RespawnIfFar_();
}

void EnemyHoming::Draw(Camera& camera) {
	if (model_)
		model_->Draw(worldTransform_, camera);
	for (auto* b : bullets_)
		b->Draw(camera);
}
