#include "Player.h"

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_.y = 1.0f; // 少し高い位置からスタート
}

void Player::Update() {
	// 1. 重力を適用
	const float kGravity = 0.02f;
	velocity_.y -= kGravity;

	// 2. 仮の移動先座標を計算
	KamataEngine::Vector3 nextPosition = worldTransform_.translation_;
	nextPosition.x += velocity_.x;
	nextPosition.y += velocity_.y;

	// 3. マップとの当たり判定
	
	// とりあえず簡易的な床と壁
	if (nextPosition.y < 1.0f) { // 地面の高さをY=1.0とする
		nextPosition.y = 1.0f;
		velocity_.y = 0.0f;
	}

	const float kWallLeft = 0.0f;
	const float kWallRight = 58.0f; // 30ブロック * 2幅 - 2
	if (nextPosition.x < kWallLeft || nextPosition.x > kWallRight) {
		velocity_.x *= -1.0f;
		nextPosition.x += velocity_.x; // 反転後の移動を少し加える
	}

	// 4. 座標を更新
	worldTransform_.translation_ = nextPosition;

	// ワールド行列を更新
	worldTransform_.TransferMatrix();
}

void Player::Draw() { model_->Draw(worldTransform_, *camera_); }