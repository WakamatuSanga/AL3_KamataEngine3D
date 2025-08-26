#include "Skydome.h"
#include "cassert"

void Skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	camera_ = camera;

	// ★ 追加：色オブジェクト初期化（デフォルトは白＝無変化）
	color_.Initialize();
	color_.SetColor({1, 1, 1, 1});
}

void Skydome::Update() { worldTransform_.TransferMatrix(); }

void Skydome::Draw() {
	// ★ 変更：色を渡す版の Draw を使う
	model_->Draw(worldTransform_, *camera_, &color_);
}
