#pragma once
#include "KamataEngine.h"

class Ground {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model);

	// 更新
	void Update();

	// 描画
	void Draw(KamataEngine::Camera& camera);

private:
	// 地面（海）のモデル
	KamataEngine::Model* model_ = nullptr;

	// ワールド変換データ（位置・大きさ・回転）
	KamataEngine::WorldTransform worldTransform_;
};