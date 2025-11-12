#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include <vector>

class EnemyManager {
public:
	void Initialize(KamataEngine::Model* enemyModel);

	void Update(const RailCamera& rc, float dt);
	void Draw(KamataEngine::Camera& camera);

	// 外から明示的に呼んでスポーンしても良いし、
	// Update 内で自動スポーン用に使ってもよい
	void SpawnStraight(float dist, float ux, float uy, float speed);
	// プレイヤーが動ける画面領域の“奥”からスポーン
	void SpawnStraightInPlayerArea(float dist, float speed);
	// GameScene から敵リストを読むため
	const std::vector<Enemy*>& GetEnemies() const { return enemies_; }

private:
	KamataEngine::Model* model_ = nullptr;
	std::vector<Enemy*> enemies_;

	// 簡単なテスト用 自動スポーンタイマー
	float spawnTimer_ = 0.0f;

	// プレイヤー移動範囲に合わせた画面内の正規化範囲（-1〜+1 のうちどこまで使うか）
	//  必要に応じて調整してOK
	float playerUxRange_ = 0.8f;   // 左右：-0.8〜+0.8
	float playerUyTop_ = 0.4f;     // 上端
	float playerUyBottom_ = -0.6f; // 下端（プレイヤーが下寄りなので少し広め）
};
