#include "RailCameraController.h"
#include <imgui.h>

using namespace KamataEngine;

void RailCameraController::Initialize(const Vector3& pos, const Vector3& rot, float fovY, float nearZ, float farZ) {
	worldTransform_.Initialize();
	worldTransform_.scale_ = {1, 1, 1};
	worldTransform_.rotation_ = rot;
	worldTransform_.translation_ = pos;
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	camera_.Initialize();
	camera_.fovAngleY = fovY;
	camera_.nearZ = nearZ;
	camera_.farZ = farZ;

	// 入力
	input_ = KamataEngine::Input::GetInstance();

	// ビュー＝カメラオブジェクトの逆行列
	camera_.matView = Inverse(worldTransform_.matWorld_);

	// ★ matView を自前で設定する設計なら TransferMatrix にする（UpdateMatrix が matView を作り直す実装の場合がある）
	camera_.TransferMatrix();
	// camera_.UpdateMatrix(); // ←もし UpdateMatrix が射影だけ更新で、matView を触らないならこれでもOK
}

void RailCameraController::Update() {
	// ---- Space を押している間だけヨー回転（右回り） ----
	/*if (input_ && input_->PushKey(DIK_SPACE)) {
		float yawPerFrame = 0.01f;
		if (input_->PushKey(DIK_LSHIFT) || input_->PushKey(DIK_RSHIFT)) {
			yawPerFrame *= 2.0f;
		}
		worldTransform_.rotation_.y += yawPerFrame;
	}*/

	// ---- 行列更新 → ビュー更新 ----
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	camera_.matView = Inverse(worldTransform_.matWorld_);
	camera_.TransferMatrix();
	// camera_.UpdateMatrix(); // ←こちらを使うなら、matView を上書きしない実装であることを確認
}




void RailCameraController::DebugGui() {
	if (ImGui::Begin("RailCamera")) {
		ImGui::Checkbox("Spin", &dbgSpin_);
		ImGui::SliderFloat("Speed (deg/s)", &dbgDegPerSec_, 1.0f, 90.0f);
		ImGui::SliderFloat("Radius", &dbgRadius_, 5.0f, 200.0f);
		ImGui::SliderFloat("Height", &dbgHeight_, -20.0f, 40.0f);
		ImGui::DragFloat3("Pivot", &dbgPivot_.x, 0.1f);

		ImGui::Separator();
		ImGui::DragFloat3("pos", &worldTransform_.translation_.x, 0.05f);
		ImGui::DragFloat3("rot", &worldTransform_.rotation_.x, 0.01f);
		ImGui::DragFloat("fovY", &camera_.fovAngleY, 0.001f, 0.1f, 1.3f);
		ImGui::DragFloat("nearZ", &camera_.nearZ, 0.01f, 0.01f, 10.0f);
		ImGui::DragFloat("farZ", &camera_.farZ, 1.0f, 10.0f, 2000.0f);
	}
	ImGui::End();
}
