#pragma once
#include "KamataEngine.h"

/// <summary>
/// 天球
/// </summary>
class Skydome {
public:
	void Initialize(KamataEngine::Model* model);
	void Update();
	void Draw(KamataEngine::Camera& camera);

	// 位置セット
	void SetPosition(const KamataEngine::Vector3& pos) { worldTransform_.translation_ = pos; }

	// ★追加：回転セット（錯覚演出用）
	void SetRotation(const KamataEngine::Vector3& rot) { worldTransform_.rotation_ = rot; }

private:
	KamataEngine::Camera camera_;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
};