#include "Player.h"
#include "EnemyManager.h"
#include "MyMath.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

Player::~Player() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();

	if (spriteReticle_)
		delete spriteReticle_;
	// ★追加：HPバースプライトの解放
	if (spriteHPBarBG_)
		delete spriteHPBarBG_;
	if (spriteHPBar_)
		delete spriteHPBar_;
}

void Player::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	input_ = Input::GetInstance();

	bulletModel_ = Model::CreateFromOBJ("playerBullet");
	if (!bulletModel_) {
		bulletModel_ = Model::Create();
	}

	// HP初期化
	hp_ = 10;
	maxHp_ = 10.0f; // 最大HPを記憶
	isDead_ = false;
	invincibilityTimer_ = 0;

	// レティクル
	uint32_t reticleTexture = TextureManager::Load("./Resources/reticle/reticle.png");
	spriteReticle_ = Sprite::Create(reticleTexture, {0, 0});
	spriteReticle_->SetAnchorPoint({0.5f, 0.5f});
	spriteReticle_->SetSize({128.0f, 128.0f});
	spriteReticle_->SetColor({1.0f, 1.0f, 1.0f, 0.5f});

	
	uint32_t barTexture = TextureManager::Load("./Resources/white/white.png");
	// 万が一読み込めなかったらレティクルで代用（エラー落ち防止）
	if (barTexture == 0)
		barTexture = reticleTexture;

	// 背景（グレー）
	spriteHPBarBG_ = Sprite::Create(barTexture, {20.0f, 680.0f}); // 左下座標
	spriteHPBarBG_->SetSize({300.0f, 30.0f});                     // 横300, 縦30
	spriteHPBarBG_->SetColor({0.3f, 0.3f, 0.3f, 0.8f});           // 半透明グレー

	// 前景（緑）
	spriteHPBar_ = Sprite::Create(barTexture, {20.0f, 680.0f});
	spriteHPBar_->SetSize({300.0f, 30.0f});
	spriteHPBar_->SetColor({0.2f, 1.0f, 0.2f, 1.0f}); // 鮮やかな緑

	seShoot_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_player_shoot.wav");
	seHit_ = Audio::GetInstance()->LoadWave("./Resources/SE/se_player_hit.wav");
}

void Player::SpawnBullet() {
	if (!bulletModel_)
		return;

	// 発射音再生
	uint32_t h = Audio::GetInstance()->PlayWave(seShoot_, false);
	Audio::GetInstance()->SetVolume(h, 0.3f);

	const float kBulletSpeed = 1.0f;

	Vector3 spawnPos{
	    worldTransform_.matWorld_.m[3][0],
	    worldTransform_.matWorld_.m[3][1],
	    worldTransform_.matWorld_.m[3][2],
	};

	Vector3 toTarget = target3DPos_ - spawnPos;
	Vector3 velocity = Normalized(toTarget) * kBulletSpeed;

	auto* b = new PlayerBullet();
	b->Initialize(bulletModel_, spawnPos, velocity);
	b->SetLifeTimeFrames(300);
	b->SetAlignToVelocity(true);
	bullets_.push_back(b);
}

float Player::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 2.0f;
	return kBase * m;
}

void Player::SetParent(const KamataEngine::WorldTransform* parent) { worldTransform_.parent_ = parent; }

void Player::OnCollision() {
	if (invincibilityTimer_ <= 0) {
		// 被弾音再生
		uint32_t h = Audio::GetInstance()->PlayWave(seHit_, false);
		Audio::GetInstance()->SetVolume(h, 0.8f);

		hp_--;
		if (hp_ <= 0) {
			hp_ = 0;
			isDead_ = true;
		} else {
			invincibilityTimer_ = kInvincibilityTime;
		}
	}
}

void Player::Update(const Camera& camera, const EnemyManager* enemyManager) {
	if (invincibilityTimer_ > 0) {
		invincibilityTimer_--;
	}

	if (isDead_)
		return;

	WorldTransformUpdate(worldTransform_);

	// --- レティクル＆照準計算 ---
	Vector2 mousePos = input_->GetMousePosition();

	if (spriteReticle_) {
		spriteReticle_->SetPosition(mousePos);
	}

	float winW = 1280.0f;
	float winH = 720.0f;

	Matrix4x4 matView = camera.matView;
	Matrix4x4 matProj = MakePerspectiveFovMatrix(camera.fovAngleY, winW / winH, camera.nearZ, camera.farZ);
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, winW, winH, 0, 1);

	Matrix4x4 matVPV = matView * matProj * matViewport;
	Matrix4x4 matInvVPV = Inverse(matVPV);

	Vector3 nearPos = Transform(Vector3(mousePos.x, mousePos.y, 0), matInvVPV);
	Vector3 farPos = Transform(Vector3(mousePos.x, mousePos.y, 1), matInvVPV);
	Vector3 rayDir = Normalized(farPos - nearPos);

	// --- ロックオン処理 ---
	Vector3 hitPos;
	bool isLocked = false;

	if (enemyManager) {
		if (enemyManager->GetReticleTarget(mousePos, matVPV, hitPos)) {
			target3DPos_ = hitPos;
			isLocked = true;
			if (spriteReticle_)
				spriteReticle_->SetColor({1.0f, 0.3f, 0.3f, 0.8f});
		}
	}

	if (!isLocked) {
		float distance = 300.0f;
		target3DPos_ = nearPos + rayDir * distance;
		if (spriteReticle_)
			spriteReticle_->SetColor({1.0f, 1.0f, 1.0f, 0.5f});
	}

	// --- 射撃 ---
	bool pressed = input_->IsTriggerMouse(0);
	bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool released = (!held && mouseHeldPrev_);
	mouseHeldPrev_ = held;

	if (pressed) {
		SpawnBullet();
		holdFrames_ = 0;
		autoFireCounter_ = 0;
	}

	if (held) {
		++holdFrames_;
		if (holdFrames_ >= autoFireDelayFrames_) {
			++autoFireCounter_;
			if (autoFireCounter_ >= autoFireIntervalFrames_) {
				SpawnBullet();
				autoFireCounter_ = 0;
			}
		}
	}

	if (released) {
		holdFrames_ = 0;
		autoFireCounter_ = 0;
	}

	// 弾更新
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		PlayerBullet* b = *it;
		b->Update();
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
}

void Player::Draw(Camera& camera) {
	if (invincibilityTimer_ > 0 && (invincibilityTimer_ / 5) % 2 == 0) {
	} else {
		if (model_) {
			model_->Draw(worldTransform_, camera);
		}
	}

	for (PlayerBullet* b : bullets_) {
		b->Draw(camera);
	}
}

void Player::DrawUI() {
	if (isDead_)
		return;

	// HPバーの描画処理
	if (spriteHPBarBG_ && spriteHPBar_) {
		// 現在のHP割合を計算 (0.0 ～ 1.0)
		float ratio = (float)hp_ / maxHp_;
		if (ratio < 0.0f)
			ratio = 0.0f;

		// バーの最大幅（初期化時に設定した300.0f）
		float maxWidth = 300.0f;
		float currentWidth = maxWidth * ratio;

		// 前景バーのサイズを更新
		spriteHPBar_->SetSize({currentWidth, 30.0f});

		// 描画（背景 → 前景 の順）
		spriteHPBarBG_->Draw();
		spriteHPBar_->Draw();
	}

	// レティクル描画
	if (spriteReticle_) {
		spriteReticle_->Draw();
	}
}