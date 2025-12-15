#pragma once
#include "Enemy.h"
#include "EnemyAimer.h"
#include "EnemyHoming.h"
#include "KamataEngine.h"
#include "Player.h"
#include <list>
#include <sstream>
#include <string>
#include <vector>

// 敵発生データ構造体
struct EnemySpawnData {
	float time;                     // 発生時間（秒）
	int type;                       // 敵の種類 (0:通常, 1:自機狙い, 2:ホーミング)
	KamataEngine::Vector3 position; // 発生座標
};

class EnemyManager {
public:
	void Initialize(Player* player);
	void Update();
	void Draw(KamataEngine::Camera& camera);
	~EnemyManager();

	void LoadEnemyData();

	// 外部参照用も vector に変更
	const std::vector<Enemy*>& GetEnemies() const { return enemies_; }
	const std::vector<EnemyAimer*>& GetAimers() const { return aimers_; }
	const std::vector<EnemyHoming*>& GetHomings() const { return homings_; }

private:
	Player* player_ = nullptr;

	// モデル
	KamataEngine::Model* enemyModel_ = nullptr;
	KamataEngine::Model* enemyAimerModel_ = nullptr;
	KamataEngine::Model* enemyHomingModel_ = nullptr;
	KamataEngine::Model* normalBulletModel_ = nullptr;
	KamataEngine::Model* homingBulletModel_ = nullptr;

	// アクティブな敵リスト：vector に変更
	std::vector<Enemy*> enemies_;
	std::vector<EnemyAimer*> aimers_;
	std::vector<EnemyHoming*> homings_;

	// スポーン待機リスト（こちらは先頭削除を行うため list のままが効率的です）
	std::list<EnemySpawnData> spawnList_;
	float timer_ = 0.0f;
};