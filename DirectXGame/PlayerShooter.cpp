#include "PlayerShooter.h"
#include "MyMath.h"
#include <Windows.h> // WinAPIで押下中補完（IsPressMouseが無い前提）

using namespace KamataEngine;

void PlayerShooter::Initialize() {
	// 必要なら初期値をここで調整
}

void PlayerShooter::SetModels(Model* bulletNormal, Model* bulletCharged) {
	modelBullet_ = bulletNormal;
	modelCharged_ = bulletCharged;
	if (bullets_)
		bullets_->SetModels(modelBullet_, modelCharged_);
}

bool PlayerShooter::IsMousePressed() const {
	// KAMATA ENGINE: 左ボタン 0
	return Input::GetInstance()->IsTriggerMouse(0);
}
bool PlayerShooter::IsMouseHeld() const {
	// エンジンに「押下中」APIが無い前提 → WinAPIで補完
	return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

void PlayerShooter::UpdateReticleWorld(const RailCamera& rc) {
	// 1) FOVから「aimDist_ 平面」の半幅/半高
	float halfW = 5.0f, halfH = 3.0f;
	rc.GetViewPlaneHalfExtents(aimDist_, halfW, halfH);

	// 2) 相対移動を平面に反映
	Input::MouseMove mv = Input::GetInstance()->GetMouseMove();
	aimOffset_.x += static_cast<float>(mv.lX) * mouseSens_ * halfW;
	aimOffset_.y += static_cast<float>(-mv.lY) * mouseSens_ * halfH;

	// 3) 画角内にクランプ
	float maxX = halfW * margin_;
	float minX = -halfW * margin_;
	float maxY = halfH * margin_;
	float minY = -halfH * margin_;
	if (aimOffset_.x > maxX)
		aimOffset_.x = maxX;
	if (aimOffset_.x < minX)
		aimOffset_.x = minX;
	if (aimOffset_.y > maxY)
		aimOffset_.y = maxY;
	if (aimOffset_.y < minY)
		aimOffset_.y = minY;

	// 4) 平面中心 + Right*x + Up*(baseY + y)
	float baseY = biasYFrac_ * halfH;
	Vector3 center = rc.ScreenPlaneCenter(aimDist_);
	reticleWorld_ = center + (rc.Right() * aimOffset_.x) + (rc.Up() * (baseY + aimOffset_.y));
}

void PlayerShooter::Update(const WorldTransform& playerWT, const RailCamera& rc, float dt) {
	if (!bullets_)
		return;

	UpdateReticleWorld(rc);

	bool held = IsMouseHeld();
	bool pressed = IsMousePressed() || (held && !heldPrev_); // 押した瞬間
	bool released = (!held && heldPrev_);
	heldPrev_ = held;

	cd_ = max(0.0f, cd_ - dt);

	if (held) {
		// チャージ蓄積
		charge_ = min(charge_ + dt, chargeMax_);

		// チャージ閾値未満 → 通常弾（押した瞬間に1発＋レート撃ち）
		if (charge_ < chargeNeed_) {
			Vector3 muzzle = playerWT.translation_ + (rc.Fwd() * 0.6f);
			Vector3 dir = Normalized(reticleWorld_ + (-(muzzle)));

			if (pressed && cd_ <= 0.0f) {
				bullets_->SpawnNormal(muzzle, dir);
				cd_ = 1.0f / rps_;
			} else if (cd_ <= 0.0f) {
				bullets_->SpawnNormal(muzzle, dir);
				cd_ = 1.0f / rps_;
			}
		}
	}

	if (released) {
		// 閾値超えならチャージ弾
		if (charge_ >= chargeNeed_) {
			Vector3 muzzle = playerWT.translation_ + (rc.Fwd() * 0.6f);
			Vector3 dir = Normalized(reticleWorld_ + (-(muzzle)));
			bullets_->SpawnCharged(muzzle, dir);
		}
		charge_ = 0.0f;
		cd_ = 0.0f;
	}

	bullets_->Update(dt);
}
