#include "PlayerShot.h"
#include "MyMath.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void PlayerShot::Initialize(Model* bulletModel) { bulletModel_ = bulletModel; }

bool PlayerShot::IsMousePressed() const {
	// KAMATA ENGINE: 左ボタン 0
	return Input::GetInstance()->IsTriggerMouse(0);
}

bool PlayerShot::IsMouseHeld() const {
	// 押しっぱ判定（IsPressMouse が無い前提なので WinAPI で補完）
	return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

void PlayerShot::UpdateReticleWorld(const RailCamera& rc) {
	// 1) FOV から aimDist_ 平面での半幅/半高を求める
	float halfW = 5.0f, halfH = 3.0f;
	rc.GetViewPlaneHalfExtents(aimDist_, halfW, halfH);

	// 2) マウス相対移動を平面上のオフセットに反映
	Input::MouseMove mv = Input::GetInstance()->GetMouseMove();
	aimOffset_.x += static_cast<float>(mv.lX) * mouseSens_ * halfW;
	aimOffset_.y += static_cast<float>(-mv.lY) * mouseSens_ * halfH;

	// 3) 画角内にクランプ
	float maxX = halfW * margin_;
	float minX = -halfW * margin_;
	float maxY = halfH * margin_;
	float minY = -halfH * margin_;
	aimOffset_.x = std::clamp(aimOffset_.x, minX, maxX);
	aimOffset_.y = std::clamp(aimOffset_.y, minY, maxY);

	// 4) 平面中心 + Right*x + Up*(baseY + y)
	float baseY = biasYFrac_ * halfH;
	Vector3 center = rc.ScreenPlaneCenter(aimDist_);
	reticleWorld_ = center + (rc.Right() * aimOffset_.x) + (rc.Up() * (baseY + aimOffset_.y));
}

// カーソルモード用：前方向を中心にしたコーン内の方向を作る
Vector3 PlayerShot::ComputeAutoDir(const WorldTransform& playerWT, const RailCamera& rc) {
	Vector3 startPos = playerWT.translation_;

	// レティクル方向
	Vector3 dirAim = Normalized(reticleWorld_ + (-startPos));

	// コーンの中心方向：レールカメラの前方向
	Vector3 base = Normalized(rc.Fwd());

	float dot = dirAim.x * base.x + dirAim.y * base.y + dirAim.z * base.z;

	if (dot < minDot_) {
		// base から離れすぎたので「baseを中心とした円錐」内に押し込む
		Vector3 perp = dirAim + (-(base * dot)); // perp = dirAim - base * dot
		float lenP = Length(perp);

		if (lenP > 1e-5f) {
			perp = perp * (1.0f / lenP);
		} else {
			// ほぼ同じ方向のときの保険
			perp = {1.0f, 0.0f, 0.0f};
		}

		float perpLen = std::sqrt(max(0.0f, 1.0f - minDot_ * minDot_));

		dirAim = base * minDot_ + perp * perpLen;
		dirAim = Normalized(dirAim);
	}

	return dirAim;
}

// 1発撃つ：完全に前方向（画面奥）へ飛ばす
void PlayerShot::SpawnBulletStraight(const WorldTransform& playerWT, const RailCamera& rc) {
	if (!bulletModel_)
		return;

	Vector3 startPos = playerWT.translation_;
	Vector3 dir = Normalized(rc.Fwd()); // 常に前方向
	Vector3 velocity = dir * bulletSpeed_;

	Bullet* b = new Bullet();
	b->Initialize(bulletModel_, startPos, velocity);
	bullets_.push_back(b);
}

// カーソルモード連射用：レティクル方向を前方向コーンに収めて撃つ
void PlayerShot::SpawnBulletAuto(const WorldTransform& playerWT, const RailCamera& rc) {
	if (!bulletModel_)
		return;

	Vector3 startPos = playerWT.translation_;
	Vector3 dir = ComputeAutoDir(playerWT, rc);
	Vector3 velocity = dir * bulletSpeed_;

	Bullet* b = new Bullet();
	b->Initialize(bulletModel_, startPos, velocity);
	bullets_.push_back(b);
}

void PlayerShot::Update(const WorldTransform& playerWT, const RailCamera& rc, float dt) {
	// --- 照準更新 ---
	UpdateReticleWorld(rc);

	auto* input = Input::GetInstance();

	// --- スペースキーでモード切り替え ---
	// DIK_SPACE が通らなければ DIKSPACE に置き換えてください
	if (input->TriggerKey(DIK_SPACE)) {
		if (mode_ == FireMode::Cursor) {
			mode_ = FireMode::Straight;
		} else {
			mode_ = FireMode::Cursor;
		}
	}

	bool held = IsMouseHeld();
	bool pressed = IsMousePressed();
	bool released = (!held && wasHeld_);
	wasHeld_ = held;

	// ボタンを離したらタイマー類リセット
	if (released) {
		holdTime_ = 0.0f;
		autoFireTimer_ = 0.0f;
	}

	// --- 1発目（単押し・長押し共通）---
	// どのモードでも「押した瞬間の1発目は真っ直ぐ奥へ」
	if (pressed) {
		SpawnBulletStraight(playerWT, rc);
		holdTime_ = 0.0f;
		autoFireTimer_ = 0.0f;
	}

	// --- 長押し（連射） ---
	if (held) {
		holdTime_ += dt;

		if (holdTime_ >= autoFireDelay_) {
			autoFireTimer_ += dt;
			if (autoFireTimer_ >= autoFireInterval_) {
				if (mode_ == FireMode::Cursor) {
					// カーソルモード：レティクル方向（前方向コーン制限あり）
					SpawnBulletAuto(playerWT, rc);
				} else {
					// ストレートモード：ずっと前方向だけ
					SpawnBulletStraight(playerWT, rc);
				}
				autoFireTimer_ = 0.0f;
			}
		}
	}

	// --- 弾の更新 & 破棄 ---
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		Bullet* b = *it;
		b->Update(dt);
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
}

void PlayerShot::Draw(Camera& camera) {
	for (Bullet* b : bullets_) {
		b->Draw(camera);
	}
}
