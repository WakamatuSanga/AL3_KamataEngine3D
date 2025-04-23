#pragma once
#include <vector>
#include"KamataEngine.h"
// ゲームシーン
class GameScene {
public:
	//3Dモデル
	KamataEngine::Model* model_ = nullptr;
	//スプライト
	KamataEngine::Sprite* spreite_ = nullptr;
	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	//カメラ
	KamataEngine::Camera camera_;
	//デバックカメラ
	/*KamataEngine::DebugCamera* debugCamera_ = nullptr;*/
	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	KamataEngine::Model* modelBlock_;
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
