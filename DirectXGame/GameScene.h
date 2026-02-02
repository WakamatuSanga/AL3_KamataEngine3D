#pragma once
#include "EnemyManager.h"
#include "Ground.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "Clouds.h"
#include "RailCameraController.h"
#include "Skydome.h"
#include <vector>
#include <string>

class GameScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~GameScene() override;

	enum class Phase {
		kWait,
		kIntro,
		kMove,
		kEnd,
	};

private:
	KamataEngine::Camera camera_;

	// プレイヤー
	Player player_;
	KamataEngine::Model* playerModel_ = nullptr;

	// 敵管理
	EnemyManager* enemyManager_ = nullptr;

	// 天球
	Skydome skydome_;
	KamataEngine::Model* modelSkydome_ = nullptr;

	// 地面
	Ground ground_;
	KamataEngine::Model* groundModel_ = nullptr;

	// 雲
	Clouds clouds_;
	KamataEngine::Model* cloudModel_ = nullptr;

	// レールカメラ
	RailCameraController railCamera_;

	// HPラベル用
	uint32_t texHP_ = 0;
	KamataEngine::Sprite* spriteHP_ = nullptr;

	// 数字モデル (3Dオブジェクト 0~9)
	KamataEngine::Model* modelNumbers_[10] = {};
	KamataEngine::Model* modelSlash_ = nullptr;

	// 数字描画用のワールドトランスフォーム
	KamataEngine::WorldTransform wtNumber_;

	// UI描画用の固定カメラ
	KamataEngine::Camera uiCamera_;

	// 3D数字描画のヘルパー関数
	// number: 表示する数値
	// position: UIカメラ空間での基準位置
	void DrawNumber3D(int number, const KamataEngine::Vector3& position);

	KamataEngine::WorldTransform playerWorldTransform_;
	KamataEngine::Vector3 playerLocalPos_ = {0, 0, 0};

	std::vector<KamataEngine::Vector3> splineControlPoints_;
	std::vector<KamataEngine::Vector3> splinePoints_;

	Phase phase_ = Phase::kWait;
	float timer_ = 0.0f;
	float splineT_ = 0.0f;
	float moveSpeed_ = 1.0f / 1800.0f;

	bool isDebugCamera_ = false;
	KamataEngine::Vector3 debugCameraRot_ = {0, 0, 0};
	KamataEngine::Vector2 preMousePos_ = {0, 0};

	void CheckAllCollisions();

	// 1フレーム前のレール位置（移動量計算用）
	KamataEngine::Vector3 preRailPos_ = {0, 0, 0};


	// BGM
	uint32_t bgmHandle_ = 0;
	uint32_t bgmVoiceHandle_ = 0;

	// 雑魚敵用SE（一括管理）
	uint32_t seEnemyHit_ = 0;
	uint32_t seEnemyDead_ = 0;
};