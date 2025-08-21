#include "CameraController.h"
#include "MyMath.h"
#include "Player.h"
#include <algorithm>

void CameraController::Initialize(Camera* camera) { camera_ = camera; }

void CameraController::Update() {

	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 02_06 スライド29枚目で追加
	const Vector3& targetVelocity = target_->GetVelocity();

	// 追従対象とオフセットからカメラの座標を計算
	destination_ = targetWorldTransform.translation_ + targetOffset_ + targetVelocity * kVelocityBias;

	// 座標補間によりゆったり追従(数学関数追加)
	camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);

	// --- ここから追加: シェイク適用 ---
	Vector3 shakeOffset = {0, 0, 0};
	if (shakeTime_ < shakeDuration_) {
		shakeTime_ += 1.0f / 60.0f;
		float t = std::clamp(1.0f - (shakeTime_ / shakeDuration_), 0.0f, 1.0f);
		float amp = shakeMagnitude_ * t;

		// 疑似ノイズ（決定論的なsinベース）で各軸を揺らす
		float s = shakeTime_ * shakeFreq_;
		shakeOffset.x = std::sin(s * 12.9898f) * amp;
		shakeOffset.y = std::sin(s * 78.2330f) * amp;
		// Zは画面奥行で見えにくいので0でもOK。必要なら有効化
		// shakeOffset.z = std::sin(s * 37.719f) * amp;
	}
	camera_->translation_ += shakeOffset;
	// --- ここまで追加 ---
	
	// 移動範囲制限 02_06 スライド34枚目
	camera_->translation_.x = max(camera_->translation_.x, destination_.x + targetMargin.left);
	camera_->translation_.x = min(camera_->translation_.x, destination_.x + targetMargin.right);
	camera_->translation_.y = max(camera_->translation_.y, destination_.y + targetMargin.bottom);
	camera_->translation_.y = min(camera_->translation_.y, destination_.y + targetMargin.top);

	// 移動範囲制限 02_06 スライド19枚目
	camera_->translation_.x = max(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = min(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = min(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = max(camera_->translation_.y, movableArea_.top);

	camera_->UpdateMatrix();
}

void CameraController::Reset() {

	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}


void CameraController::AddShake(float magnitude, float duration) {
	shakeMagnitude_ = max(0.0f, magnitude);
	shakeDuration_ = max(0.0f, duration);
	shakeTime_ = 0.0f;
}
