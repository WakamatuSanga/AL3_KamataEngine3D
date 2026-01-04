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
	// bulletModel_ はエンジン側が共有管理なら delete 不要。必要な設計ならここで delete。
}

void Player::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	input_ = Input::GetInstance();

	bulletModel_ = Model::Create();

	// HP初期化
	hp_ = 10;
	isDead_ = false;
	invincibilityTimer_ = 0;
}

// 弾生成
void Player::SpawnBullet() {
	if (!bulletModel_)
		return;

	const float kBulletSpeed = 1.0f;

	// ワールド行列の 3x3 部分で前方向(+Z)を求める
	Vector3 forwardWorld = TransformNormal({0, 0, 1}, worldTransform_.matWorld_);
	forwardWorld = Normalized(forwardWorld);

	// プレイヤーのワールド座標
	Vector3 spawnPos{
	    worldTransform_.matWorld_.m[3][0],
	    worldTransform_.matWorld_.m[3][1],
	    worldTransform_.matWorld_.m[3][2],
	};

	Vector3 velocity = forwardWorld * kBulletSpeed;

	auto* b = new PlayerBullet();
	b->Initialize(bulletModel_, spawnPos, velocity);
	bullets_.push_back(b);
}

float Player::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f; // モデル素の半径（見た目に合わせて調整）
	return kBase * m;
}

void Player::SetParent(const KamataEngine::WorldTransform* parent) { worldTransform_.parent_ = parent; }

void Player::OnCollision() {
	// 無敵時間中でなければダメージ
	if (invincibilityTimer_ <= 0) {
		hp_--;
		if (hp_ <= 0) {
			hp_ = 0;
			isDead_ = true;
		} else {
			invincibilityTimer_ = kInvincibilityTime;
		}
	}
}

void Player::Update() {
	// 無敵時間の更新
	if (invincibilityTimer_ > 0) {
		invincibilityTimer_--;
	}

	// 死亡していたら更新しない（あるいは爆発演出などへ）
	if (isDead_)
		return;

	// 移動（WASD）
	Vector3 move{0.0f, 0.0f, 0.0f};
	const float kSpeed = 0.2f;
	if (input_->PushKey(DIK_A))
		move.x -= kSpeed;
	if (input_->PushKey(DIK_D))
		move.x += kSpeed;
	if (input_->PushKey(DIK_W))
		move.y += kSpeed;
	if (input_->PushKey(DIK_S))
		move.y -= kSpeed;
	worldTransform_.translation_ += move;
	// 親（レールカメラ）込みのワールド行列を先に更新
	WorldTransformUpdate(worldTransform_);
	// 射撃（左クリック：単発＋長押し連射）
	bool pressed = input_->IsTriggerMouse(0);                 // 押した瞬間
	bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; // 押下中
	bool released = (!held && mouseHeldPrev_);
	mouseHeldPrev_ = held;

	if (pressed) {
		SpawnBullet();
		holdFrames_ = 0;
		autoFireCounter_ = 0; // リセット
	}

	if (held) {
		++holdFrames_;
		if (holdFrames_ >= autoFireDelayFrames_) {
			++autoFireCounter_;
			if (autoFireCounter_ >= autoFireIntervalFrames_) {
				SpawnBullet();
				autoFireCounter_ = 0; // 間隔で発射
			}
		}
	}

	if (released) {
		holdFrames_ = 0;
		autoFireCounter_ = 0; // リセット
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

	// 点滅処理（無敵時間中）
	if (model_) {
		if (invincibilityTimer_ > 0) {
			// 点滅：偶数フレームなどで描画・非描画を切り替えるか、色を変える
			// ここでは簡易的に色を赤くする等
			// model_->SetColor({1, 0, 0, 1}); // エンジンに機能があれば
		}
	}

}

void Player::Draw(Camera& camera) {
	// 無敵時間中は点滅させてみる（簡易実装：数フレームおきに描画スキップ）
	if (invincibilityTimer_ > 0 && (invincibilityTimer_ / 5) % 2 == 0) {
		// スキップ
	} else {
		if (model_) {
			model_->Draw(worldTransform_, camera);
		}
	}

	for (PlayerBullet* b : bullets_) {
		b->Draw(camera);
	}
}