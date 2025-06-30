#pragma once
#include "KamataEngine.h"

class WorldTransform {
public:
	KamataEngine::Vector3 scale_{1.0f, 1.0f, 1.0f};
	KamataEngine::Vector3 rotation_{0.0f, 0.0f, 0.0f};
	KamataEngine::Vector3 translation_{0.0f, 0.0f, 0.0f};

	KamataEngine::Matrix4x4 matWorld_{};

	void Initialize();     // 初期化（必要なら）
	void TransferMatrix(); // GPU転送（既存の関数と整合性持たせる）
	void UpdateMatrix();   // アフィン行列の再計算（あれば）

	// その他の補助関数も追加可能
};