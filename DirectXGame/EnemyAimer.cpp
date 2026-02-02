#include "EnemyAimer.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

EnemyAimer::~EnemyAimer() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
	// bulletModel_ は適宜削除
}

void EnemyAimer::Initialize(Model* model, Player* player) {
	model_ = model;
	player_ = player;
	isDead_ = false;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 40.0f};

	bulletModel_ = Model::CreateFromOBJ("enemyBullet");
	shotTimer_ = 0;

	// SEロード
	seShoot_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_enemy_shoot.wav");
}

void EnemyAimer::FireAimedBullet() {
	if (!bulletModel_ || !player_)
		return;

	// 再生
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.3f);

	const float kBulletSpeed = 0.6f;
	Vector3 enemyPos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 dir = playerPos;
	dir -= enemyPos;
	dir = Normalized(dir);

	Vector3 vel = dir * kBulletSpeed;

	auto* b = new EnemyBullet();
	b->Initialize(bulletModel_, enemyPos, vel);
	b->SetAlignToVelocity(true);
	bullets_.push_back(b);
}

float EnemyAimer::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f;
	return kBase * m;
}

void EnemyAimer::Update() {
	if (isDead_)
		return;

	// 手前へ移動
	worldTransform_.translation_.z += moveSpeedZ_;

	// 射撃
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

	// 弾更新
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

	// 画面外で死亡
	const auto& p = worldTransform_.translation_;
	if (p.z < -40.0f || std::fabs(p.x) > 40.0f || std::fabs(p.y) > 40.0f) {
		isDead_ = true;
	}
}

void EnemyAimer::Draw(Camera& camera) {
	if (isDead_)
		return;
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
	for (EnemyBullet* b : bullets_) {
		b->Draw(camera);
	}
}