#pragma once
#include "KamataEngine.h"
#include "Player.h" 
using namespace KamataEngine;
// ゲームシーン
class GameScene {
public:
	// カメラ
	KamataEngine::Camera camera_;

	// プレイヤー
	Player* player_ = nullptr;
	KamataEngine::Model* playerModel_ = nullptr;

	// 床
	KamataEngine::WorldTransform floorWorldTransform_;
	KamataEngine::Model* floorModel_ = nullptr;

	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// デストラクタ
	~GameScene();
};