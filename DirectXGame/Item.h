#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include <numbers>
#include <cmath>  

using namespace KamataEngine;

class Item {

public:
	void Initialize(Model* model, Camera* camera, const Vector3& pos) {
		model_ = model;
		camera_ = camera;
		wt_.Initialize();
		wt_.translation_ = pos;
		baseY_ = pos.y;  
		WorldTransformUpdate(wt_);
	}
	void Update() {
		
		  // 回転（固定フレーム）
		constexpr float dt = 1.0f / 60.0f;
		spinAngle_ += spinSpeed_ * dt;

		// 角度を 0〜2π にラップ（std::numbers を使う版）
		const float TWO_PI = 2.0f * std::numbers::pi_v<float>; // ← MyMath.h の PI を使うなら 2.0f * PI
		if (spinAngle_ > TWO_PI) {
			spinAngle_ -= TWO_PI;
		}

		wt_.rotation_.y = spinAngle_;


		   // --- ふわふわ上下（追加） ---
		bobTimer_ += bobSpeed_ * dt; // ラジアンで加算
		if (bobTimer_ > TWO_PI)
			bobTimer_ -= TWO_PI;
		float bobOffset = std::sin(bobTimer_) * bobAmplitude_;
		wt_.translation_.y = baseY_ + bobOffset;

		WorldTransformUpdate(wt_);
	}
	void Draw() {
		if (collected_)
			return;
		model_->Draw(wt_, *camera_);
	}
	// Item.h の Item クラス内（public: に）
	void SetSpinSpeed(float radPerSec) { spinSpeed_ = radPerSec; }
	void SetBob(float amplitude, float speedRadPerSec) {
		bobAmplitude_ = amplitude;
		bobSpeed_ = speedRadPerSec;
	}
	//void SetBobPhase(float rad) { bobTimer_ = rad; }
	bool IsCollected() const { return collected_; }
	void Collect() { collected_ = true; }

	AABB GetAABB() const {
		Vector3 c;
		c.x = wt_.matWorld_.m[3][0];
		c.y = wt_.matWorld_.m[3][1];
		c.z = wt_.matWorld_.m[3][2];
		AABB a;
		const float r = kSize * 0.5f;
		a.min = {c.x - r, c.y - r, c.z - r};
		a.max = {c.x + r, c.y + r, c.z + r};
		return a;
	}
	// お好みで回転速度を外から変えたい場合
	

	/*  void SetBob(float amplitude, float speedRadPerSec) {
		bobAmplitude_ = amplitude;
		bobSpeed_ = speedRadPerSec;
	}*/
	void SetBobPhase(float rad) { bobTimer_ = rad; } // ばらけさせる用

private:
	WorldTransform wt_{};
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;
	bool collected_ = false;

	static inline const float kSize = 0.6f; // 当たりサイズ

		// 回転用
	float spinAngle_ = 0.0f; // 現在角度[rad]
	float spinSpeed_ = 2.0f; // 1秒あたりの回転速度[rad/s]（お好みで）

	// --- ふわふわ上下用 ---
	float baseY_ = 0.0f;         // 初期Y（中心）
	float bobTimer_ = 0.0f;      // 経過
	float bobSpeed_ = 2.0f;      // 1秒あたりの角速度[rad/s]
	float bobAmplitude_ = 0.25f; // 振幅（上下量）
};
