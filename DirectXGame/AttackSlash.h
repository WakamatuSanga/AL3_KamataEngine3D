#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include <algorithm>

    using namespace KamataEngine;

class AttackSlash {
public:
	// 寿命seconds, 開始スケール, 終了スケール, 姿勢補正（Z回転等）
	void Initialize(
	    Model* model, Camera* camera, const Vector3& pos, float yaw, float lifetime = 0.15f, const Vector3& scaleStart = {1.0f, 1.0f, 1.0f}, const Vector3& scaleEnd = {1.6f, 0.8f, 1.0f},
	    float roll = 0.0f) {
		model_ = model;
		camera_ = camera;
		wt_.Initialize();
		wt_.translation_ = pos;
		wt_.rotation_.y = yaw;  // 向き（プレイヤーのY回転）
		wt_.rotation_.z = roll; // 斬撃に少し傾きをつけたい場合
		scaleStart_ = scaleStart;
		scaleEnd_ = scaleEnd;
		life_ = lifetime;
		timer_ = 0.0f;

		color_.Initialize();
		// 半透明っぽい白→最後にフェードアウト
		tint_ = {1, 1, 1, 1};
	}

	void Update() {
		if (Finished())
			return;
		timer_ += 1.0f / 60.0f;
		const float denom = (std::max)(life_, 0.0001f); // ← マクロ衝突を避けるため () を付けて呼ぶ
		const float t = std::clamp(timer_ / denom, 0.0f, 1.0f);


		// スケールを補間＆若干前に押し出す
		wt_.scale_.x = (1.0f - t) * scaleStart_.x + t * scaleEnd_.x;
		wt_.scale_.y = (1.0f - t) * scaleStart_.y + t * scaleEnd_.y;
		wt_.scale_.z = (1.0f - t) * scaleStart_.z + t * scaleEnd_.z;

		// 斬撃が前進する感じ（向きの前方へ少し移動）
		const float push = 0.08f; // 調整可
		wt_.translation_.x += push * std::cos(wt_.rotation_.y);
		wt_.translation_.z -= push * std::sin(wt_.rotation_.y);

		// フェードアウト
		tint_.w = std::clamp(1.0f - t, 0.0f, 1.0f);
		color_.SetColor(tint_);

		WorldTransformUpdate(wt_);
	}

	void Draw() {
		if (Finished())
			return;
		// モデル側が ObjectColor に対応していれば色が乗る
		model_->Draw(wt_, *camera_, &color_);
	}

	bool Finished() const { return timer_ >= life_; }

private:
	WorldTransform wt_{};
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

	Vector3 scaleStart_{1, 1, 1}, scaleEnd_{1, 1, 1};
	float life_ = 0.15f;
	float timer_ = 0.0f;

	ObjectColor color_{};
	Vector4 tint_{1, 1, 1, 1};
};
