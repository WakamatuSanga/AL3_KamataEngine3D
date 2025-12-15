#include "Ground.h"
#include "MyMath.h"

using namespace KamataEngine;

void Ground::Initialize(Model* model) {
	model_ = model;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// ★サイズ調整
	// 箱（海）なので、プレイヤーを包み込むくらい大きくします
	// ステージの広さに合わせて数値を調整してください
	worldTransform_.scale_ = {200.0f, 200.0f, 200.0f};

	// ★位置調整
	// カメラの原点に合わせて配置します
	worldTransform_.translation_ = {0.0f, -100.0f, 0.0f};

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
}

void Ground::Update() {
	// もし「海をゆらゆら動かしたい」とか「スクロールさせたい」場合は
	// ここで translation_ や rotation_ をいじります。
	// とりあえず今回は固定でOKです。

	// 行列を確定させる
	WorldTransformUpdate(worldTransform_);
}

void Ground::Draw(Camera& camera) {
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
}