#pragma once
#include "EnemyManager.h"
#include "Ground.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "Player.h"
#include "RailCameraController.h"
#include "Skydome.h"
#include <vector>

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

	// 敵管理はこれ一本に
	EnemyManager* enemyManager_ = nullptr;

	// 天球
	Skydome skydome_;
	KamataEngine::Model* modelSkydome_ = nullptr;

	// 地面
	Ground ground_;
	KamataEngine::Model* groundModel_ = nullptr;

	// レールカメラ
	RailCameraController railCamera_;

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
};