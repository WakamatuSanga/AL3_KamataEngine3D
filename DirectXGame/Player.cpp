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
	worldTransform_.translation_ = {0.0f, 0.0f, -35.0f};

	input_ = Input::GetInstance();

	bulletModel_ = Model::Create();
}

// 弾生成
void Player::SpawnBullet() {
	if (!bulletModel_)
		return;

	const float kBulletSpeed = 1.0f;

	// 回転行列
	Matrix4x4 rx = MakeRotateXMatrix(worldTransform_.rotation_.x);
	Matrix4x4 ry = MakeRotateYMatrix(worldTransform_.rotation_.y);
	Matrix4x4 rz = MakeRotateZMatrix(worldTransform_.rotation_.z);
	Matrix4x4 r = rz * rx * ry;

	// ★ 並進無視で前方向を回す
	Vector3 forwardWorld = TransformNormal({0, 0, 1}, r);
	forwardWorld = Normalized(forwardWorld);

	Vector3 velocity = forwardWorld * kBulletSpeed;

	auto* b = new PlayerBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, velocity);
	bullets_.push_back(b);
}

float Player::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f; // モデル素の半径（見た目に合わせて調整）
	return kBase * m;
}

void Player::Update() {
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

	// 射撃（左クリック：単発＋長押し連射）
	bool pressed = input_->IsTriggerMouse(0);                 // 押した瞬間
	bool held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; // 押下中
	bool released = (!held && mouseHeldPrev_);
	mouseHeldPrev_ = held;

	if (pressed) {
		SpawnBullet();
		holdFrames_ = 0;
		autoFireCounter_ = 0; // ★ リセット
	}

	if (held) {
		++holdFrames_;
		if (holdFrames_ >= autoFireDelayFrames_) {
			++autoFireCounter_;
			if (autoFireCounter_ >= autoFireIntervalFrames_) {
				SpawnBullet();
				autoFireCounter_ = 0; // ★ 間隔で発射
			}
		}
	}

	if (released) {
		holdFrames_ = 0;
		autoFireCounter_ = 0; // ★ リセット
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

	// ワールド行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Player::Draw(Camera& camera) {
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
	for (PlayerBullet* b : bullets_) {
		b->Draw(camera);
	}
}
