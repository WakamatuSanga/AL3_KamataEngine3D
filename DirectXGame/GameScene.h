#pragma once
#include "KamataEngine.h"
#include "Player.h" 
#include "MapChipField.h"
#include <vector>
using namespace KamataEngine;
// ゲームシーン
class GameScene {
public:
	// カメラ
	KamataEngine::Camera camera_;

	// プレイヤー
	Player* player_ = nullptr;
	KamataEngine::Model* playerModel_ = nullptr;

	
	// マップチップ
	MapChipField* mapChipField_ = nullptr;
	KamataEngine::Model* blockModel_ = nullptr;
	// WorldTransformをたくさん管理するためのvector
	std::vector<KamataEngine::WorldTransform*> blockWorldTransforms_;

	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// デストラクタ
	~GameScene();
};