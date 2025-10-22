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

	// 移動範囲制限 02_06 スライド34枚目
	using std::clamp;
	camera_->translation_.x = clamp(camera_->translation_.x, destination_.x + targetMargin.left, destination_.x + targetMargin.right);
	camera_->translation_.y = clamp(camera_->translation_.y, destination_.y + targetMargin.bottom, destination_.y + targetMargin.top);

	camera_->translation_.x = clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.y = clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);


	camera_->UpdateMatrix();
}

void CameraController::Reset() {

	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}
