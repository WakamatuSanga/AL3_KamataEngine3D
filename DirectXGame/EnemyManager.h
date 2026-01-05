#pragma once
#include "Enemy.h"
#include "EnemyAimer.h"
#include "EnemyFollow.h"
#include "EnemyHoming.h"
#include "KamataEngine.h"
#include "Player.h"
#include <list>
#include <sstream>
#include <string>
#include <vector>

struct EnemySpawnData {
	float time;
	int type;
	KamataEngine::Vector3 position;
};

class EnemyManager {
public:
	EnemyManager() = default;
	~EnemyManager();

	void Initialize(Player* player);
	void Update(const KamataEngine::Matrix4x4& cameraMat, const KamataEngine::Vector3& cameraRot);
	void Draw(KamataEngine::Camera& camera);
	void LoadEnemyData();

	// クリア判定用：全てのFollow敵（出現待ち含む）がいなくなったか？
	bool IsAllFollowEnemiesDead() const;

	const std::vector<Enemy*>& GetEnemies() const { return enemies_; }
	const std::vector<EnemyAimer*>& GetAimers() const { return aimers_; }
	const std::vector<EnemyHoming*>& GetHomings() const { return homings_; }
	const std::vector<EnemyFollow*>& GetFollows() const { return follows_; }

private:
	Player* player_ = nullptr;

	KamataEngine::Model* enemyModel_ = nullptr;
	KamataEngine::Model* enemyAimerModel_ = nullptr;
	KamataEngine::Model* enemyHomingModel_ = nullptr;
	KamataEngine::Model* enemyFollowModel_ = nullptr;

	KamataEngine::Model* normalBulletModel_ = nullptr;
	KamataEngine::Model* homingBulletModel_ = nullptr;

	std::vector<Enemy*> enemies_;
	std::vector<EnemyAimer*> aimers_;
	std::vector<EnemyHoming*> homings_;
	std::vector<EnemyFollow*> follows_;

	std::list<EnemySpawnData> spawnList_;
	float timer_ = 0.0f;
};