#pragma once
#include "KamataEngine.h"
#include "RailCamera.h"
#include <algorithm>
#include <cmath>
using namespace KamataEngine;

class PlayerMovement {
public:
	void Initialize() {
		moveSpeed_ = 8.0f;
		lerpK_ = 0.15f;
		tiltZ_ = 0.10f;
		screenPlaneDist_ = 8.0f; // ★ カメラから前方へ何mの平面に乗せるか
		margin_ = 0.85f;         // ★ 画角の何%内でクランプするか
		biasYFrac_ = -0.55f;     // ★ 画面下寄せ（-0.55で下寄り）
		frame_ = 0;
		lastA_ = lastD_ = -1000;
		rollFrames_ = 0;
		offset_ = {0, 0};
		vel_ = {0, 0};
	}

	// 設定用
	void SetViewPlaneDist(float d) { screenPlaneDist_ = d; }
	void SetScreenBiasY(float frac) { biasYFrac_ = frac; } // -0.6f で下寄り

	// return: 見た目ロール角
	float Update(WorldTransform& wt, const RailCamera& rc) {
		frame_++;
		auto* in = Input::GetInstance();

		// 入力
		float ix = 0.0f, iy = 0.0f;
		if (in->PushKey(DIK_A)) {
			ix -= 1.0f;
		}
		if (in->PushKey(DIK_D)) {
			ix += 1.0f;
		}
		if (in->PushKey(DIK_W)) {
			iy += 1.0f;
		}
		if (in->PushKey(DIK_S)) {
			iy -= 1.0f;
		}

		// ダブルタップ（A/D）
		if (in->TriggerKey(DIK_A)) {
			if (frame_ - lastA_ <= doubleTapFrames_) {
				rollFrames_ = kRollFrames_;
			}
				lastA_ = frame_;
			
		}
		if (in->TriggerKey(DIK_D)) {
			if (frame_ - lastD_ <= doubleTapFrames_) {
				rollFrames_ = kRollFrames_;
			}
				lastD_ = frame_;
			
		}
		if (rollFrames_ > 0) {
			--rollFrames_;
		}

		// 可視範囲（FOV）から半幅/半高を算出
		float halfW = 5.0f, halfH = 3.0f;
		rc.GetViewPlaneHalfExtents(screenPlaneDist_, halfW, halfH);

		// 画面下寄せのベース量（半高に対する割合）
		float baseY = biasYFrac_ * halfH;

		// クランプ範囲（ベースを加味して offset_ を制限）
		float maxX = halfW * margin_;
		float minX = -halfW * margin_;
		float maxY = halfH * margin_ - baseY;
		float minY = -halfH * margin_ - baseY;

		// 慣性 & クランプ
		vel_.x += (ix * moveSpeed_ - vel_.x) * lerpK_;
		vel_.y += (iy * moveSpeed_ - vel_.y) * lerpK_;
		offset_.x = std::clamp(offset_.x + vel_.x * (1.0f / 60.0f), minX, maxX);
		offset_.y = std::clamp(offset_.y + vel_.y * (1.0f / 60.0f), minY, maxY);

		// 位置：カメラ前方の平面上
		Vector3 center = rc.ScreenPlaneCenter(screenPlaneDist_);
		wt.translation_ = center + (rc.Right() * offset_.x) + (rc.Up() * (offset_.y + baseY));

		// 向き（進行方向へYaw）
		float yaw = std::atan2(rc.Fwd().x, rc.Fwd().z);
		wt.rotation_.y = yaw;

		// 見た目ロール（左右傾き + ロール演出）
		float lookRoll = -offset_.x * tiltZ_;
		if (rollFrames_ > 0) {
			float t = 1.0f - (rollFrames_ / (float)kRollFrames_);
			lookRoll = 2.0f * 3.14159265f * t;
		}
		wt.rotation_.z = lookRoll;

		wt.matWorld_ = MakeAffineMatrix(wt.scale_, wt.rotation_, wt.translation_);
		wt.TransferMatrix();
		return lookRoll;
	}

private:
	Vector2 offset_{0, 0};
	Vector2 vel_{0, 0};

	float moveSpeed_ = 8.0f, lerpK_ = 0.15f, tiltZ_ = 0.10f;

	// ★ 画面保持用パラメータ
	float screenPlaneDist_ = 8.0f; // カメラから前方の距離
	float margin_ = 0.85f;         // 画角の内側に留める割合
	float biasYFrac_ = -0.55f;     // 画面下寄せ（-1..+1 目安）

	// ダブルタップ
	int frame_ = 0, lastA_ = -1000, lastD_ = -1000;
	static constexpr int doubleTapFrames_ = 12;
	int rollFrames_ = 0;
	static constexpr int kRollFrames_ = 30;
};
