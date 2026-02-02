#include "EnemyHoming.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>
using namespace KamataEngine;

EnemyHoming::~EnemyHoming() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
	// bulletModel_ は適宜
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
	isDead_ = false;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 40.0f};

	bulletModel_ = Model::CreateFromOBJ("homingBullet");
	if (!bulletModel_)
		bulletModel_ = Model::Create();

	shotTimer_ = 0;
	// ★SEロード
	seShoot_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_enemy_shoot.wav");
}

void EnemyHoming::FireHomingBullet_() {
	if (!bulletModel_ || !player_)
		return;

	auto* b = new EnemyHomingBullet();
	KamataEngine::Vector3 spawn = worldTransform_.translation_;
	b->Initialize(bulletModel_, spawn, bulletSpeed_, turnRate_, player_);
	b->SetLifeTime(90);
	bullets_.push_back(b);
}

void EnemyHoming::Update() {
	if (isDead_)
		return;

	// 移動
	worldTransform_.translation_.z += moveSpeedZ_;

	// 発射
	++shotTimer_;
	if (shotTimer_ >= shotInterval_) {
		// 再生
		uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
		Audio::GetInstance()->SetVolume(h, 0.3f);
		shotTimer_ = 0;
		FireHomingBullet_();
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 弾更新
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

	// 画面外死亡
	const auto& p = worldTransform_.translation_;
	if (p.z < -40.0f || std::fabs(p.x) > 40.0f || std::fabs(p.y) > 40.0f) {
		isDead_ = true;
	}
}

void EnemyHoming::Draw(Camera& camera) {
	if (isDead_)
		return;
	if (model_)
		model_->Draw(worldTransform_, camera);
	for (auto* b : bullets_)
		b->Draw(camera);
}