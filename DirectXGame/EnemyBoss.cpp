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

	// スプライト解放
	if (hpBarBG_)
		delete hpBarBG_;
	if (hpBar_)
		delete hpBar_;
	// ★追加
	if (warningArea_)
		delete warningArea_;
}

void EnemyBoss::Initialize(Model* bossModel, Model* bulletModel, Model* homingBulletModel, Player* player) {
	model_ = bossModel;
	bulletModel_ = bulletModel;
	homingBulletModel_ = homingBulletModel;
	player_ = player;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {kModelScale_, kModelScale_, kModelScale_};
	worldTransform_.rotation_ = {0.0f, 3.14159f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 200.0f};

	// HP設定
	hp_ = 500;
	maxHp_ = 500.0f;

	isDead_ = false;
	phase_ = Phase::Approach;
	phaseTimer_ = 0;
	moveTimer_ = 0;

	// --- UI作成 ---
	uint32_t whiteTex = TextureManager::Load("./Resources/white/white.png");
	if (whiteTex == 0)
		whiteTex = TextureManager::Load("./Resources/reticle/reticle.png");

	// HPバー
	hpBarBG_ = Sprite::Create(whiteTex, {240.0f, 20.0f});
	hpBarBG_->SetSize({800.0f, 20.0f});
	hpBarBG_->SetColor({0.1f, 0.1f, 0.1f, 0.8f});

	hpBar_ = Sprite::Create(whiteTex, {240.0f, 20.0f});
	hpBar_->SetSize({800.0f, 20.0f});
	hpBar_->SetColor({1.0f, 0.2f, 0.2f, 1.0f});

	// 警告エリア作成
	warningArea_ = Sprite::Create(whiteTex, {0.0f, 0.0f});
	// サイズは描画時に調整します
	warningArea_->SetColor({1.0f, 0.0f, 0.0f, 0.3f}); // 薄い赤

	// SEロード
	seShoot_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_boss_shoot.wav");
	seHit_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_boss_hit.wav");
	seDead_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_boss_dead.wav");
}

void EnemyBoss::Update(const Vector3& cameraPos) {
	if (isDead_)
		return;

	phaseTimer_++;

	// --- ボスの移動制御 ---
	if (phase_ == Phase::AttackBeamRight) {
		if (phaseTimer_ < 90) {
			Vector3 safePos = {-30.0f, 0.0f, cameraPos.z + 60.0f};
			worldTransform_.translation_.x += (safePos.x - worldTransform_.translation_.x) * 0.05f;
			worldTransform_.translation_.y += (safePos.y - worldTransform_.translation_.y) * 0.05f;
			worldTransform_.translation_.z += (safePos.z - worldTransform_.translation_.z) * 0.05f;
			worldTransform_.rotation_.y = 3.14159f + 0.3f;
		}
	} else if (phase_ == Phase::AttackBeamLeft) {
		if (phaseTimer_ < 90) {
			Vector3 safePos = {30.0f, 0.0f, cameraPos.z + 60.0f};
			worldTransform_.translation_.x += (safePos.x - worldTransform_.translation_.x) * 0.05f;
			worldTransform_.translation_.y += (safePos.y - worldTransform_.translation_.y) * 0.05f;
			worldTransform_.translation_.z += (safePos.z - worldTransform_.translation_.z) * 0.05f;
			worldTransform_.rotation_.y = 3.14159f - 0.3f;
		}
	} else if (phase_ != Phase::Approach) {
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
		if (phaseTimer_ % 8 == 0)
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
		if (phaseTimer_ % 30 == 0)
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
		if (phaseTimer_ >= 90)
			FireBeam(true);
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;

	case Phase::AttackBeamLeft:
		if (phaseTimer_ >= 90)
			FireBeam(false);
		if (phaseTimer_ >= 150) {
			phase_ = Phase::Standby;
			phaseTimer_ = 0;
		}
		break;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

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

// UI描画
void EnemyBoss::DrawUI() {
	if (isDead_)
		return;

	// --- HPバー ---
	float ratio = (float)hp_ / maxHp_;
	if (ratio < 0.0f)
		ratio = 0.0f;

	if (hpBarBG_ && hpBar_) {
		hpBar_->SetSize({800.0f * ratio, 20.0f});
		hpBarBG_->Draw();
		hpBar_->Draw();
	}

	// --- ★追加：ビーム警告エリア ---
	// 攻撃までの予備動作期間（0～90フレーム）だけ表示
	// かつ、攻撃中も少し残すなら条件を変える（今回は警告のみ）
	if (warningArea_) {
		// ビーム右 (画面の右半分)
		if (phase_ == Phase::AttackBeamRight && phaseTimer_ < 90) {
			// 640x720 (画面幅の半分)
			// 座標: 中央(640) ～ 右端(1280)
			warningArea_->SetPosition({640.0f, 0.0f});
			warningArea_->SetSize({640.0f, 720.0f});

			// 点滅演出 (alphaを波打たせる)
			float alpha = 0.3f + std::sin((float)phaseTimer_ * 0.2f) * 0.2f;
			warningArea_->SetColor({1.0f, 0.0f, 0.0f, alpha});

			warningArea_->Draw();
		}
		// ビーム左 (画面の左半分)
		else if (phase_ == Phase::AttackBeamLeft && phaseTimer_ < 90) {
			// 0x720 (画面幅の半分)
			// 座標: 左端(0) ～ 中央(640)
			warningArea_->SetPosition({0.0f, 0.0f});
			warningArea_->SetSize({640.0f, 720.0f});

			float alpha = 0.3f + std::sin((float)phaseTimer_ * 0.2f) * 0.2f;
			warningArea_->SetColor({1.0f, 0.0f, 0.0f, alpha});

			warningArea_->Draw();
		}
	}
}

void EnemyBoss::OnCollision() {
	if (isDead_)
		return;
	// 被弾音再生
	uint32_t h = Audio::GetInstance()->PlayWave(seHit_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);

	hp_--;
	if (hp_ <= 0) {
		hp_ = 0;
		isDead_ = true;
		// 撃破音再生
		Audio::GetInstance()->PlayWave(seDead_, false);
	}
}

float EnemyBoss::GetCollisionRadius() const { return 2.5f * kModelScale_; }

// --- 攻撃関数群 ---

void EnemyBoss::FireAimedBullet() {
	if (!player_ || !bulletModel_)
		return;

	// 発射音
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);

	Vector3 pos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 toPlayer = playerPos - pos;
	toPlayer = Normalized(toPlayer);

	int ways = 5;
	float angleStep = 12.0f * (3.14159f / 180.0f);
	float baseAngle = std::atan2(toPlayer.x, toPlayer.z);

	for (int i = 0; i < ways; ++i) {
		float angleOffset = (float)(i - ways / 2) * angleStep;
		angleOffset += RandF(-0.05f, 0.05f);

		float currentAngle = baseAngle + angleOffset;

		Vector3 vel;
		vel.x = std::sin(currentAngle);
		vel.y = toPlayer.y;
		vel.z = std::cos(currentAngle);

		for (int s = 0; s < 2; ++s) {
			float speed = (s == 0) ? 1.2f : 0.8f;
			Vector3 finalVel = Normalized(vel) * speed;

			EnemyBullet* b = new EnemyBullet();
			b->Initialize(bulletModel_, pos, finalVel);
			b->SetScale(1.5f);
			b->SetAlignToVelocity(true);
			bullets_.push_back(b);
		}
	}
}

void EnemyBoss::FireHomingBullet() {
	if (!player_ || !homingBulletModel_)
		return;

	// 発射音
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);

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

void EnemyBoss::FireSpreadBullet() {
	if (!bulletModel_)
		return;
	// 発射音
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);
	Vector3 pos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 toPlayer = playerPos - pos;
	Vector3 baseDir = Normalized(toPlayer);

	int num = 32;
	float spreadAngle = 60.0f * (3.14159f / 180.0f);
	float baseAngle = std::atan2(baseDir.x, baseDir.z);

	for (int i = 0; i < num; ++i) {
		float angleOffset = RandF(-spreadAngle, spreadAngle);
		float currentAngle = baseAngle + angleOffset;
		float yOffset = RandF(-0.3f, 0.3f);

		Vector3 vel;
		vel.x = std::sin(currentAngle);
		vel.y = baseDir.y + yOffset;
		vel.z = std::cos(currentAngle);

		float speed = RandF(0.8f, 1.1f);
		vel = Normalized(vel) * speed;

		EnemyBullet* b = new EnemyBullet();
		b->Initialize(bulletModel_, pos, vel);
		b->SetScale(1.0f);
		b->SetAlignToVelocity(true);
		bullets_.push_back(b);
	}
}

void EnemyBoss::FireSpiralBullet() {
	if (!bulletModel_)
		return;
	// 発射音
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);
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

void EnemyBoss::FireDanmaku() {
	if (!bulletModel_)
		return;
	// 発射音
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);
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

void EnemyBoss::FireTouhouHoming() {
	if (!player_ || !homingBulletModel_)
		return;
	// 発射音
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.4f);
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

void EnemyBoss::FireBeam(bool isRight) {
	if (!bulletModel_)
		return;
	//// 発射音
	//uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	//Audio::GetInstance()->SetVolume(h, 0.4f);
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