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
	if (spriteReticle_) {
		delete spriteReticle_;
	}
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

	hp_ = 10;
	isDead_ = false;
	invincibilityTimer_ = 0;

	// レティクル設定
	uint32_t reticleTexture = TextureManager::Load("./Resources/reticle/reticle.png");
	spriteReticle_ = Sprite::Create(reticleTexture, {0, 0});
	spriteReticle_->SetAnchorPoint({0.5f, 0.5f});
	spriteReticle_->SetSize({128.0f, 128.0f});
	spriteReticle_->SetColor({1.0f, 1.0f, 1.0f, 0.5f});
}

// 弾生成
void Player::SpawnBullet() {
	if (!bulletModel_)
		return;

	const float kBulletSpeed = 1.0f;

	// 自機の現在位置（ワールド座標）
	// ※Update内で更新された最新の行列から取得
	Vector3 spawnPos{
	    worldTransform_.matWorld_.m[3][0],
	    worldTransform_.matWorld_.m[3][1],
	    worldTransform_.matWorld_.m[3][2],
	};

	// レティクルが指す3D位置へのベクトル
	Vector3 toTarget = target3DPos_ - spawnPos;
	Vector3 velocity = Normalized(toTarget) * kBulletSpeed;

	auto* b = new PlayerBullet();
	b->Initialize(bulletModel_, spawnPos, velocity);

	// 画面奥まで届くように寿命を長く設定
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
		hp_--;
		if (hp_ <= 0) {
			hp_ = 0;
			isDead_ = true;
		} else {
			invincibilityTimer_ = kInvincibilityTime;
		}
	}
}

// Update関数
void Player::Update(const Camera& camera) {
	if (invincibilityTimer_ > 0) {
		invincibilityTimer_--;
	}

	if (isDead_)
		return;

	// ★修正ポイント1：重複していた移動処理（WASD入力）を削除しました
	// 移動は GameScene 側で制御され、worldTransform_ に反映済みです。
	// ここでは最新の位置情報を使って行列を確定させます。
	WorldTransformUpdate(worldTransform_);

	// --- レティクル＆照準計算 ---
	Vector2 mousePos = input_->GetMousePosition();
	if (spriteReticle_) {
		spriteReticle_->SetPosition(mousePos);
	}

	// 1. ビューポート逆行列の作成
	float winW = 1280.0f;
	float winH = 720.0f;

	Matrix4x4 matView = camera.matView;
	Matrix4x4 matProj = MakePerspectiveFovMatrix(camera.fovAngleY, winW / winH, camera.nearZ, camera.farZ);
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, winW, winH, 0, 1);

	Matrix4x4 matVPV = matView * matProj * matViewport;
	Matrix4x4 matInvVPV = Inverse(matVPV);

	// 2. マウス位置からのレイ（光線）を作成
	Vector3 nearPos = Transform(Vector3(mousePos.x, mousePos.y, 0), matInvVPV);
	Vector3 farPos = Transform(Vector3(mousePos.x, mousePos.y, 1), matInvVPV);
	Vector3 rayDir = Normalized(farPos - nearPos);

	// 3. ターゲット位置を決定
	// ★距離を200.0fに設定（ボスの初期位置付近）
	// これにより、画面端を狙った際の角度ズレを最小限に抑えます
	float distance = 500.0f;
	target3DPos_ = nearPos + rayDir * distance;

	// --- 射撃処理 ---
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
		// 点滅中は描画スキップ
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
	if (spriteReticle_) {
		spriteReticle_->Draw();
	}
}