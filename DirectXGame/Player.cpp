#include "Player.h"

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_.y = 1.0f; // 少し高い位置からスタート
}

void Player::Update() {
	// 1. 重力を適用
	const float kGravity = 0.01f;
	velocity_.y -= kGravity;

	// 2. 座標を更新
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;

	// 3. 床との当たり判定
	if (worldTransform_.translation_.y < 0.0f) {
		worldTransform_.translation_.y = 0.0f;
		velocity_.y = 0.0f; // 落下を止める
	}

	// 4. 壁との当たり判定
	const float kWallLeft = -10.0f; // 左の壁のX座標
	const float kWallRight = 10.0f; // 右の壁のX座標

	if (worldTransform_.translation_.x < kWallLeft || worldTransform_.translation_.x > kWallRight) {
		// 移動方向を反転させる
		velocity_.x *= -1.0f;
	}

	// ワールド行列を更新
	worldTransform_.TransferMatrix();
}

void Player::Draw() { model_->Draw(worldTransform_, *camera_); }