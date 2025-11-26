#include "Skydome.h"
#include "MyMath.h"
using namespace KamataEngine;

void Skydome::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	// 半径は大きめ（OBJが半径1想定ならスケールで直径を作る）
	worldTransform_.scale_ = {200.0f, 200.0f, 200.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};
	camera_.farZ = 1000.0f;
	// 今日は固定配置でOK（追従したい場合はカメラ位置を入れるだけ）
	WorldTransformUpdate(worldTransform_);
}

void Skydome::Update() {
	// 位置固定なら行列転送だけでOK
	worldTransform_.TransferMatrix();
}

void Skydome::Draw(Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
}
