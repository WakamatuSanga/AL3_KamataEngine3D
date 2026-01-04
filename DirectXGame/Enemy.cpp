#include "Enemy.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

Enemy::~Enemy() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
	// bulletModel_ はManager管理ならdeleteしないが、個別生成ならdelete
	// 今回はManagerがモデルを持っているので、ここでは弾の管理だけ注意
}

void Enemy::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	// 位置は SetPosition で後から設定されるので適当でOK
	worldTransform_.translation_ = {0.0f, 0.0f, 25.0f};

	approachVelocity_ = {0.0f, 0.0f, -0.2f};
	leaveVelocity_ = {0.3f, 0.2f, -0.1f};
	phase_ = Phase::Approach;
	isDead_ = false;

	// 弾用モデル（テクスチャ無しのシンプルなやつ）
	// ★ 本来はManagerから貰う方が良いですが、既存維持で生成
	bulletModel_ = Model::CreateFromOBJ("enemyBullet");
}

void Enemy::FireBullet() {
	if (!bulletModel_)
		return;

	const float kBulletSpeed = 0.5f;
	Vector3 vel{0.0f, 0.0f, -kBulletSpeed};

	EnemyBullet* b = new EnemyBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, vel);
	b->SetLifeTime(90);
	bullets_.push_back(b);
}

float Enemy::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f;
	return kBase * m;
}

void Enemy::UpdateApproach() {
	worldTransform_.translation_ += approachVelocity_;

	// 一定位置まで来たら離脱へ
	if (worldTransform_.translation_.z < -30.0f) {
		phase_ = Phase::Leave;
	}

	// 一定間隔で発射
	++shotTimer_;
	if (shotTimer_ >= shotInterval_) {
		shotTimer_ = 0;
		FireBullet();
	}
}

void Enemy::UpdateLeave() {
	worldTransform_.translation_ += leaveVelocity_;

	// 画面外に出たら「死亡」扱いにする（Managerが削除してくれる）
	if (worldTransform_.translation_.z < -40.0f || std::fabsf(worldTransform_.translation_.x) > 40.0f || std::fabsf(worldTransform_.translation_.y) > 40.0f) {
		isDead_ = true;
	}
}

void Enemy::Update() {
	if (isDead_)
		return;

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

	// 弾の更新
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
	if (isDead_)
		return;
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
	for (EnemyBullet* b : bullets_) {
		b->Draw(camera);
	}
}