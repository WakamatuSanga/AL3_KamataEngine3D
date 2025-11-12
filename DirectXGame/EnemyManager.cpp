#include "EnemyManager.h"
#include <cstdlib>

using namespace KamataEngine;

void EnemyManager::Initialize(Model* enemyModel) {
	model_ = enemyModel;
	spawnTimer_ = 0.0f;
}

void EnemyManager::SpawnStraight(float dist, float ux, float uy, float speed) {
	if (!model_) {
		return;
	}

	Enemy* e = new Enemy();
	e->Initialize(model_, dist, ux, uy, speed);
	enemies_.push_back(e);
}

// プレイヤー領域の奥からスポーン
void EnemyManager::SpawnStraightInPlayerArea(float dist, float speed) {
	// ux : [-playerUxRange_, +playerUxRange_]
	float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // 0〜1
	float ux = -playerUxRange_ + (2.0f * playerUxRange_) * rx;

	// uy : [playerUyBottom_, playerUyTop_]
	float ry = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	float uy = playerUyBottom_ + (playerUyTop_ - playerUyBottom_) * ry;

	SpawnStraight(dist, ux, uy, speed);
}

void EnemyManager::Update(const RailCamera& rc, float dt) {
	// --- テスト用：一定間隔で敵を自動スポーン ---
	spawnTimer_ -= dt;
	if (spawnTimer_ <= 0.0f) {
		spawnTimer_ = 4.0f; // 4秒ごとに出現

		// プレイヤーの動ける画面内領域の奥から3体スポーン
		SpawnStraightInPlayerArea(100.0f, 15.0f);
		SpawnStraightInPlayerArea(100.0f, 15.0f);
		SpawnStraightInPlayerArea(100.0f, 15.0f);
	}

	// --- 各敵を更新＆消滅処理 ---
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		Enemy* e = *it;
		e->Update(rc, dt);
		if (e->IsDead()) {
			delete e;
			it = enemies_.erase(it);
		} else {
			++it;
		}
	}
}

void EnemyManager::Draw(Camera& camera) {
	for (Enemy* e : enemies_) {
		e->Draw(camera);
	}
}
