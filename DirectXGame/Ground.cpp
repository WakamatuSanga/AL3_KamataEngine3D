#include "Ground.h"
#include "MyMath.h"

using namespace KamataEngine;

void Ground::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	// スケールは大きめに
	worldTransform_.scale_ = {50.0f, 50.0f, 50.0f};

	// 回転初期化
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	// 位置初期化
	worldTransform_.translation_ = {0.0f, -520.0f, 0.0f};

	// 蓄積回転行列を単位行列で初期化
	matRotationAccumulated_ = MakeIdentityMatrix();

	// 初回の行列計算
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Ground::Update(float rotationSpeedX) {
	// 1. 回転軸は常に「カメラの右方向（ローカルX軸）」
	Vector3 axisX = {1.0f, 0.0f, 0.0f};

	// 2. 回転行列の生成（手前に回す）
	Matrix4x4 matRotDelta = MakeRotateAxisAngle(axisX, -rotationSpeedX);

	// 3. 蓄積回転に合成
	matRotationAccumulated_ *= matRotDelta;

	// 4. 行列の組み立て
	// Scale * AccumulatedRotation * Translate
	// ※SetPositionでセットされた translation_ を使用してスライド移動
	Matrix4x4 matScale = MakeScaleMatrix(worldTransform_.scale_);
	Matrix4x4 matTrans = MakeTranslateMatrix(worldTransform_.translation_);

	worldTransform_.matWorld_ = matScale * matRotationAccumulated_ * matTrans;
	worldTransform_.TransferMatrix();
}

void Ground::Draw(Camera& camera) {
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
}

void Ground::SetPosition(const Vector3& pos) { worldTransform_.translation_ = pos; }