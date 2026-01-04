#pragma once
#include "Enemy.h"
#include "EnemyAimer.h"
#include "EnemyHoming.h"
#include "EnemyManager.h"
#include "Ground.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "RailCameraController.h"
#include "Skydome.h"
#include <vector>

// IScene を継承させる
class GameScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~GameScene() override;
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

	// 敵管理（EnemyManagerへ移行する場合はここを差し替えますが、今回は既存維持＋Managerも使用可能にしておきます）
	EnemyManager* enemyManager_ = nullptr;

	// 既存の敵コード（EnemyManagerを使うなら不要になりますが、移行期間として残します）
	// 今回はEnemyManagerを使用するように初期化で調整します

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

	KamataEngine::WorldTransform playerWorldTransform_;
	KamataEngine::Vector3 playerLocalPos_ = {0, 0, 0};
	// ▼ スプライン制御点・描画用頂点
	std::vector<KamataEngine::Vector3> splineControlPoints_;
	std::vector<KamataEngine::Vector3> splinePoints_; // Catmull-Rom でサンプルした点たち

	// フェーズ管理
	Phase phase_ = Phase::kWait;
	float timer_ = 0.0f;
	// レールカメラ移動用
	float splineT_ = 0.0f;             // レール上の進行度 (0.0 ～ 1.0)
	float moveSpeed_ = 1.0f / 1800.0f; // 移動スピード (例: 600フレームで1周)

	// ▼▼▼ デバッグカメラ用 ▼▼▼
	bool isDebugCamera_ = false;                       // デバッグモードON/OFFフラグ
	KamataEngine::Vector3 debugCameraRot_ = {0, 0, 0}; // デバッグ時のカメラ角度
	KamataEngine::Vector2 preMousePos_ = {0, 0};       // マウスの座標保存用
};