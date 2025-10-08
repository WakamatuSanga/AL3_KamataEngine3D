#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;

class Player {
public:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 移動速度
	KamataEngine::Vector3 velocity_ = {0.05f, 0.0f, 0.0f};

	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);
	// 更新
	void Update();
	// 描画
	void Draw();
};