#include "Skydome.h"
#include "MyMath.h"
using namespace KamataEngine;

void Skydome::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	// 半径は大きめ
	worldTransform_.scale_ = {200.0f, 200.0f, 200.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	WorldTransformUpdate(worldTransform_);
}

void Skydome::Update() {
	// 位置・回転はGameSceneからSetPosition/SetRotationで指定される
	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}

void Skydome::Draw(Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
}