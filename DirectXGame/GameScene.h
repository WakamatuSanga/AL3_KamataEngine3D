#pragma once
#include "CameraController.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "skydome.h"
#include <vector>

// ゲームシーンクラス
class GameScene {
public:
	~GameScene();
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	void GenerateBlocks();

private:
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;
	// スプライト
	KamataEngine::Sprite* sprite_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	// カメラ
	KamataEngine::Camera camera_;
	// サウンドデータハンドル
	uint32_t soundDataHandle_ = 0;
	// 音声再生ハンドル
	uint32_t voiceHandle_ = 0;

	// ここから02_01からの追加
	//  プレイヤー
	Player* player_ = nullptr;
	// プレイヤーモデル
	KamataEngine::Model* player_model_ = nullptr;

	// ブロックモデル
	KamataEngine::Model* block_model_ = nullptr;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 02_03 天球
	Skydome* skydome_ = nullptr;
	Model* modelSkydome_ = nullptr;

	// 02_04 マップチップフィールド
	MapChipField* mapChipField_;

	// 02_06カメラ移動
	CameraController* CController_ = nullptr;

	// 02_09 10枚目 エネミークラス
	Enemy* enemy_ = nullptr;
	// 02_09 10枚目 エネミーモデル
	KamataEngine::Model* enemy_model_ = nullptr;
};