#pragma once
#include "IScene.h"
#include "KamataEngine.h"

class TitleScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~TitleScene() override;

private:
	// タイトルロゴ的なモデル
	KamataEngine::Model* modelTitle_ = nullptr;
	KamataEngine::WorldTransform wtTitle_;

	// 賑やかし用（プレイヤーモデルなど）
	KamataEngine::Model* modelChar_ = nullptr;
	KamataEngine::WorldTransform wtChar_;

	// カメラ
	KamataEngine::Camera camera_;
};