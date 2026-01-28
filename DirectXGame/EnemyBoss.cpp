#include "EnemyBoss.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

// 乱数ヘルパー
static float RandF(float minVal, float maxVal) {
	float r = (float)rand() / RAND_MAX;
	return minVal + r * (maxVal - minVal);
}

EnemyBoss::~EnemyBoss() {
	for (auto* b : bullets_)
		delete b;
	for (auto* b : homingBullets_)
		delete b;
	bullets_.clear();
	homingBullets_.clear();
}

void EnemyBoss::Initialize(Model* bossModel, Model* bulletModel, Model* homingBulletModel, Player* player) {
	model_ = bossModel;
	bulletModel_ = bulletModel;
	homingBulletModel_ = homingBulletModel;
	player_ = player;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {kModelScale_, kModelScale_, kModelScale_};
	// 初期状態で180度回転（正面を向く）
	worldTransform_.rotation_ = {0.0f, 3.14159f, 0.0f};

	worldTransform_.translation_ = {0.0f, 0.0f, 200.0f};

	// HP設定
	hp_ = 1000;

	isDead_ = false;
	phase_ = Phase::Approach;
	phaseTimer_ = 0;
	moveTimer_ = 0;
}

void EnemyBoss::Update(const Vector3& cameraPos) {
	if (isDead_)
		return;

	phaseTimer_++;

	// --- ボスの移動制御 ---
	if (phase_ == Phase::AttackBeamRight) {
		// 右攻撃の警告期間：ボスは左へ退避
		if (phaseTimer_ < 90) {
			Vector3 safePos = {-30.0f, 0.0f, cameraPos.z + 60.0f};
			worldTransform_.translation_.x += (safePos.x - worldTransform_.translation_.x) * 0.05f;
			worldTransform_.translation_.y += (safePos.y - worldTransform_.translation_.y) * 0.05f;
			worldTransform_.translation_.z += (safePos.z - worldTransform_.translation_.z) * 0.05f;

			worldTransform_.rotation_.y = 3.14159f + 0.3f;
		}
	} else if (phase_ == Phase::AttackBeamLeft) {
		// 左攻撃の警告期間：ボスは右へ退避
		if (phaseTimer_ < 90) {
			Vector3 safePos = {30.0f, 0.0f, cameraPos.z + 60.0f};
			worldTransform_.translation_.x += (safePos.x - worldTransform_.translation_.x) * 0.05f;
			worldTransform_.translation_.y += (safePos.y - worldTransform_.translation_.y) * 0.05f;
			worldTransform_.translation_.z += (safePos.z - worldTransform_.translation_.z) * 0.05f;

			worldTransform_.rotation_.y = 3.14159f - 0.3f;
		}
	} else if (phase_ != Phase::Approach) {
		// 通常移動
		moveTimer_++;
		if (moveTimer_ >= 180) {
			DecideNextPosition(cameraPos);
			moveTimer_ = 0;
		}

		worldTransform_.translation_.x += (targetPos_.x - worldTransform_.translation_.x) * 0.02f;
		worldTransform_.translation_.y += (targetPos_.y - worldTransform_.translation_.y) * 0.02f;
		worldTransform_.translation_.z += (targetPos_.z - worldTransform_.translation_.z) * 0.02f;

		float tiltX = (targetPos_.y - worldTransform_.translation_.y) * 0.01f;
		float tiltY = (targetPos_.x - worldTransform_.translation_.x) * 0.01f;
		worldTransform_.rotation_.x = tiltX;
		worldTransform_.rotation_.y = -tiltY + 3.14159f;

	} else {
		// 登場
		Vector3 startPos = {0.0f, 50.0f, cameraPos.z + 150.0f};
		Vector3 endPos = {0.0f, 0.0f, cameraPos.z + 60.0f};

		float t = (float)phaseTimer_ / 180.0f;
		if (t > 1.0f)
			t = 1.0f;

		float e = 1.0f - std::pow(1.0f - t, 3.0f);

		worldTransform_.translation_.x = startPos.x + (endPos.x - startPos.x) * e;
		worldTransform_.translation_.y = startPos.y + (endPos.y - startPos.y) * e;
		worldTransform_.translation_.z = startPos.z + (endPos.z - startPos.z) * e;

		worldTransform_.rotation_.y = 3.14159f;

		if (phaseTimer_ >= 180) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
			DecideNextPosition(cameraPos);
		}
	}

	// --- 攻撃ロジック ---
	switch (phase_) {
	case Phase::Standby:
		if (phaseTimer_ >= 60) {
			// 攻撃パターン抽選 (8種類)
			int r = rand() % 8;
			if (r == 0)
				phase_ = Phase::AttackAime;
			else if (r == 1)
				phase_ = Phase::AttackHoming;
			else if (r == 2)
				phase_ = Phase::AttackSpread;
			else if (r == 3)
				phase_ = Phase::AttackSpiral;
			else if (r == 4)
				phase_ = Phase::AttackDanmaku;
			else if (r == 5)
				phase_ = Phase::AttackTouhou;
			else if (r == 6)
				phase_ = Phase::AttackBeamRight;
			else
				phase_ = Phase::AttackBeamLeft;

			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackAime:
		if (phaseTimer_ % 10 == 0)
			FireAimedBullet();
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackHoming:
		if (phaseTimer_ % 50 == 0)
			FireHomingBullet();
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackSpread:
		if (phaseTimer_ % 40 == 0)
			FireSpreadBullet();
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackSpiral:
		if (phaseTimer_ % 5 == 0)
			FireSpiralBullet();
		if (phaseTimer_ >= 180) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackDanmaku:
		if (phaseTimer_ % 4 == 0)
			FireDanmaku();
		if (phaseTimer_ >= 240) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackTouhou:
		if (phaseTimer_ % 60 == 0)
			FireTouhouHoming();
		if (phaseTimer_ >= 180) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackBeamRight:
		if (phaseTimer_ >= 90) {
			FireBeam(true); // Right
		}
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackBeamLeft:
		if (phaseTimer_ >= 90) {
			FireBeam(false); // Left
		}
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 弾更新
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		(*it)->Update();
		if ((*it)->IsDead()) {
			delete *it;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
	for (auto it = homingBullets_.begin(); it != homingBullets_.end();) {
		(*it)->Update();
		if ((*it)->IsDead()) {
			delete *it;
			it = homingBullets_.erase(it);
		} else {
			++it;
		}
	}
}

void EnemyBoss::DecideNextPosition(const Vector3& cameraPos) {
	float targetZ = cameraPos.z + RandF(40.0f, 70.0f);
	float targetX = RandF(-25.0f, 25.0f);
	float targetY = RandF(-15.0f, 15.0f);

	targetPos_ = {targetX, targetY, targetZ};
}

void EnemyBoss::Draw(Camera& camera) {
	if (isDead_)
		return;
	if (model_)
		model_->Draw(worldTransform_, camera);

	for (auto* b : bullets_)
		b->Draw(camera);
	for (auto* b : homingBullets_)
		b->Draw(camera);
}

void EnemyBoss::OnCollision() {
	if (isDead_)
		return;
	hp_--;
	if (hp_ <= 0) {
		hp_ = 0;
		isDead_ = true;
	}
}

float EnemyBoss::GetCollisionRadius() const { return 2.5f * kModelScale_; }

// --- 攻撃関数群 ---

// 5-WAY狙い撃ち
void EnemyBoss::FireAimedBullet() {
	if (!player_ || !bulletModel_)
		return;
	Vector3 pos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 toPlayer = playerPos - pos;
	toPlayer = Normalized(toPlayer);

	int ways = 5;
	float angleStep = 10.0f * (3.14159f / 180.0f);
	float baseAngle = std::atan2(toPlayer.x, toPlayer.z);

	for (int i = 0; i < ways; ++i) {
		float angleOffset = (float)(i - ways / 2) * angleStep;
		float currentAngle = baseAngle + angleOffset;

		Vector3 vel;
		vel.x = std::sin(currentAngle);
		vel.y = toPlayer.y;
		vel.z = std::cos(currentAngle);

		vel = Normalized(vel) * 1.2f;

		EnemyBullet* b = new EnemyBullet();
		b->Initialize(bulletModel_, pos, vel);
		b->SetScale(1.5f);
		b->SetAlignToVelocity(true);
		bullets_.push_back(b);
	}
}

// 通常ホーミング
void EnemyBoss::FireHomingBullet() {
	if (!player_ || !homingBulletModel_)
		return;
	Vector3 pos = worldTransform_.translation_;

	for (int i = 0; i < 8; ++i) {
		float angle = (float)i * (6.28f / 8.0f);
		float spread = 5.0f;
		Vector3 offset = {std::cos(angle) * spread, std::sin(angle) * spread, 0.0f};

		EnemyHomingBullet* b = new EnemyHomingBullet();
		b->Initialize(homingBulletModel_, pos + offset, 0.7f, 0.06f, player_);
		b->SetLifeTime(300);
		homingBullets_.push_back(b);
	}
}

// 高密度拡散弾
void EnemyBoss::FireSpreadBullet() {
	if (!bulletModel_)
		return;
	Vector3 pos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 toPlayer = playerPos - pos;
	Vector3 baseDir = Normalized(toPlayer);

	// ★修正済み：未使用変数 dist を削除
	// float dist = Length(toPlayer);

	int num = 32;
	float spreadAngle = 45.0f * (3.14159f / 180.0f);
	float baseAngle = std::atan2(baseDir.x, baseDir.z);

	for (int i = 0; i < num; ++i) {
		float angleOffset = RandF(-spreadAngle, spreadAngle);
		float currentAngle = baseAngle + angleOffset;
		float yOffset = RandF(-0.2f, 0.2f);

		Vector3 vel;
		vel.x = std::sin(currentAngle);
		vel.y = baseDir.y + yOffset;
		vel.z = std::cos(currentAngle);

		vel = Normalized(vel) * 0.9f;

		EnemyBullet* b = new EnemyBullet();
		b->Initialize(bulletModel_, pos, vel);
		b->SetScale(1.0f);
		b->SetAlignToVelocity(true);
		bullets_.push_back(b);
	}
}

// 螺旋弾
void EnemyBoss::FireSpiralBullet() {
	if (!bulletModel_)
		return;
	Vector3 pos = worldTransform_.translation_;

	float angle = (float)phaseTimer_ * 0.4f;

	for (int i = 0; i < 4; ++i) {
		float a = angle + (float)i * (6.28f / 4.0f);
		Vector3 vel = {std::cos(a), std::sin(a), -0.8f};
		vel = Normalized(vel) * 1.0f;

		EnemyBullet* b = new EnemyBullet();
		b->Initialize(bulletModel_, pos, vel);
		b->SetScale(1.2f);
		b->SetAlignToVelocity(true);
		bullets_.push_back(b);
	}
}

// 花火弾幕
void EnemyBoss::FireDanmaku() {
	if (!bulletModel_)
		return;
	Vector3 pos = worldTransform_.translation_;

	int ways = 5;
	float baseAngle = (float)phaseTimer_ * 0.15f;

	for (int i = 0; i < ways; ++i) {
		float angle = baseAngle + (float)i * (6.28f / (float)ways);

		Vector3 vel;
		vel.x = std::cos(angle);
		vel.y = std::sin(angle);
		vel.z = -0.3f;

		vel = Normalized(vel) * 0.6f;

		EnemyBullet* b = new EnemyBullet();
		b->Initialize(bulletModel_, pos, vel);
		b->SetScale(0.8f);
		b->SetAlignToVelocity(true);
		bullets_.push_back(b);
	}
}

// 東方風全方位ホーミング
void EnemyBoss::FireTouhouHoming() {
	if (!player_ || !homingBulletModel_)
		return;
	Vector3 pos = worldTransform_.translation_;

	int num = 16;
	for (int i = 0; i < num; ++i) {
		float angle = (float)i * (6.28318f / (float)num);

		float radius = 10.0f;
		Vector3 offset = {std::sin(angle) * radius, std::cos(angle) * radius, 0.0f};
		Vector3 spawnPos = pos + offset;

		EnemyHomingBullet* b = new EnemyHomingBullet();
		b->Initialize(homingBulletModel_, spawnPos, 0.6f, 0.04f, player_);
		b->SetLifeTime(400);
		homingBullets_.push_back(b);
	}
}

// 左右ビーム攻撃
void EnemyBoss::FireBeam(bool isRight) {
	if (!bulletModel_)
		return;

	// Right: 0 ～ 50 / Left : -50 ～ 0
	float minX = isRight ? 0.0f : -50.0f;
	float maxX = isRight ? 50.0f : 0.0f;

	for (int i = 0; i < 5; ++i) {
		float spawnX = RandF(minX, maxX);
		float spawnY = RandF(-20.0f, 20.0f);
		float spawnZ = worldTransform_.translation_.z;

		Vector3 vel = {0.0f, 0.0f, -1.5f};

		EnemyBullet* b = new EnemyBullet();
		b->Initialize(bulletModel_, {spawnX, spawnY, spawnZ}, vel);

		b->SetScale(2.0f);
		b->SetAlignToVelocity(true);

		bullets_.push_back(b);
	}
}