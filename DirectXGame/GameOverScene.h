#pragma once
#include "IScene.h"
#include "KamataEngine.h"
#include "Skydome.h"

class GameOverScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~GameOverScene() override;

private:
	KamataEngine::Camera camera_;

	// ゲームオーバー文字モデル
	KamataEngine::Model* modelText_ = nullptr;
	KamataEngine::WorldTransform wtText_;

	// 壊れたプレイヤー（あるいは転がっている）
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::WorldTransform wtPlayer_;

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome skydome_;
};