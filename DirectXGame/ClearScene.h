#pragma once
#include "IScene.h"
#include "KamataEngine.h"
#include "Skydome.h"

class ClearScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~ClearScene() override;

private:
	KamataEngine::Camera camera_;

	// クリア表示用モデル
	KamataEngine::Model* modelText_ = nullptr;
	KamataEngine::WorldTransform wtText_;

	// プレイヤーモデル
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::WorldTransform wtPlayer_;

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome skydome_;
};