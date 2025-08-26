#pragma once
#include "KamataEngine.h"
class Skydome {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	void Update();

	void Draw();

	void SetColor(const KamataEngine::Vector4& c) { color_.SetColor(c); }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	uint32_t textureHandle_ = 0u;

	 KamataEngine::ObjectColor color_;
};