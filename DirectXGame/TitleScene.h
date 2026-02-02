#pragma once
#include "IScene.h"
#include "KamataEngine.h"
#include "Skydome.h"

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

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome skydome_;
	// BGM用ハンドル
	uint32_t bgmHandle_ = 0;      // データ
	uint32_t bgmVoiceHandle_ = 0; // 再生インスタンス
};