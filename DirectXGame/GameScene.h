#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include "EnemyAimer.h"
#include "EnemyHoming.h"
#include "Skydome.h"
class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();
	~GameScene();

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

	//ホーミング
	 EnemyHoming* enemyHoming_ = nullptr;
	KamataEngine::Model* enemyHomingModel_ = nullptr;

	// 天球
	Skydome skydome_;
	KamataEngine::Model* modelSkydome_ = nullptr;

	// 総当たり判定
	void CheckAllCollisions();
};
