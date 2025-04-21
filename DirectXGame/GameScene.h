#pragma once
#include"KamataEngine.h"
#include"Player.h"
// ゲームシーン
class GameScene {
public:
	//3Dモデル
	KamataEngine::Model* model_ = nullptr;
	////スプライト
	//KamataEngine::Sprite* spreite_ = nullptr;
	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	//カメラ
	KamataEngine::Camera camera_;
	////デバックカメラ
	//KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Player* player_ = nullptr;


	//初期化
	void Initialize();

	//更新
	void Update();

	//描画
	void Draw();


	~GameScene();

private:
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;
};
