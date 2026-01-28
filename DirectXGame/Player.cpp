#include "Player.h"
#include "MyMath.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

Player::~Player() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
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
	isDead_ = false;
	invincibilityTimer_ = 0;
}

// 弾生成
void Player::SpawnBullet() {
	if (!bulletModel_)
		return;

	// 自機の位置から発射
	Vector3 spawnPos = GetPosition();
	// 自機の前方へ
	Vector3 velocity = GetForwardDir() * 1.5f; // 弾速

	PlayerBullet* newBullet = new PlayerBullet();
	newBullet->Initialize(bulletModel_, spawnPos, velocity);
	newBullet->SetLifeTimeFrames(120); // 2秒で消滅

	bullets_.push_back(newBullet);
}

void Player::Update() {
	if (isDead_)
		return;

	// 無敵時間の更新
	if (invincibilityTimer_ > 0) {
		invincibilityTimer_--;
	}

	// --- 移動処理（GameScene側で制御している場合はここは最低限でOK） ---
	// 今回は GameScene で入力を取って位置を入れているので、ここでは行列更新を確実に行う

	// 親（レールカメラ）込みのワールド行列を更新
	WorldTransformUpdate(worldTransform_);

	// --- 射撃処理 ---
	bool pressed = input_->IsTriggerMouse(0);                 // 押した瞬間
	bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; // 押下中
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

	// 弾更新＆削除
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
	if (isDead_)
		return;

	// 無敵時間中は点滅させる（描画したりしなかったり）
	if (invincibilityTimer_ > 0 && (invincibilityTimer_ % 4 < 2)) {
		// 描画しない
	} else {
		if (model_)
			model_->Draw(worldTransform_, camera);
	}

	// 弾の描画
	for (auto* b : bullets_) {
		b->Draw(camera);
	}
}

// ★追加：衝突時の処理
void Player::OnCollision() {
	if (isDead_)
		return;

	// 無敵時間中でなければダメージ
	if (invincibilityTimer_ <= 0) {
		hp_--; // ダメージを1受ける
		if (hp_ <= 0) {
			hp_ = 0;
			isDead_ = true;
		} else {
			// 無敵時間を設定（60フレーム＝1秒）
			invincibilityTimer_ = 60;
		}
	}
}

float Player::GetCollisionRadius() const { return 1.0f; }

void Player::SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }