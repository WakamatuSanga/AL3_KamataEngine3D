#pragma once
#include "Enemy.h"
#include "EnemyAimer.h"
#include "EnemyHoming.h"
#include "Ground.h"
#include "KamataEngine.h"
#include "Player.h"
#include "RailCameraController.h"
#include "Skydome.h"
#include <vector>
class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();
	~GameScene();
	// フェーズの定義
	enum class Phase {
		kWait,  // 0: 待機（開始前）
		kIntro, // 1: 演出など（今回はスキップしてすぐ移動でもOK）
		kMove,  // 2: レール移動（ここがメイン）
		kEnd,   // 3: 終了
	};

private:
	KamataEngine::Camera camera_;

	// プレイヤー
	Player player_;
	KamataEngine::Model* playerModel_ = nullptr;

	// 敵
	Enemy* enemy_ = nullptr;
	KamataEngine::Model* enemyModel_ = nullptr;

	// 自機狙い敵
	EnemyAimer* enemyAimer_ = nullptr;
	KamataEngine::Model* enemyAimerModel_ = nullptr;

	// ホーミング
	EnemyHoming* enemyHoming_ = nullptr;
	KamataEngine::Model* enemyHomingModel_ = nullptr;

	// 天球
	Skydome skydome_;
	KamataEngine::Model* modelSkydome_ = nullptr;

	// 地面
	Ground ground_;
	KamataEngine::Model* groundModel_ = nullptr;

	// レールカメラ
	RailCameraController railCamera_;

	// 総当たり判定
	void CheckAllCollisions();

	// ▼ スプライン制御点・描画用頂点
	std::vector<KamataEngine::Vector3> splineControlPoints_;
	std::vector<KamataEngine::Vector3> splinePoints_; // Catmull-Rom でサンプルした点たち

	// フェーズ管理
	Phase phase_ = Phase::kWait;
	float timer_ = 0.0f;
	// レールカメラ移動用
	float splineT_ = 0.0f;            // レール上の進行度 (0.0 ～ 1.0)
	float moveSpeed_ = 1.0f / 600.0f; // 移動スピード (例: 600フレームで1周)
};
