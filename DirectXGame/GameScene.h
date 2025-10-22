#pragma once
#include <vector>
#include"KamataEngine.h"
#include "Spline.h"
#include "RailCamera.h"
#include "Player.h"
using namespace KamataEngine;

// ゲームシーン
class GameScene {
public:
	//3Dモデル
	Model* model_ = nullptr;
	//スプライト
	Sprite* spreite_ = nullptr;
	//ワールドトランスフォーム
	WorldTransform worldTransform_;
	//カメラ
	Camera camera_;
	//デバックカメラ
	/*DebugCamera* debugCamera_ = nullptr;*/
	bool isDebugCameraActive_ = false;
	DebugCamera* debugCamera_ = nullptr;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	Spline spline_;
	RailCamera railCam_;
	Player player_;
	Model* modelPlayer_;

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
