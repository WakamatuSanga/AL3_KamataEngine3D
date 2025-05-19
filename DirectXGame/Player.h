
#pragma once
#include "KamataEngine.h"
#include <vector>

// 自キャラ
class Player {
private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0u;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformPlayer_;

public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);
	// 更新
	void Update();
	// 描画
	void Draw();
};
