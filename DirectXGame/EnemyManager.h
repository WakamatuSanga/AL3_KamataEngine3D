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
	// コンストラクタ・デストラクタ
	EnemyManager() = default;
	~EnemyManager();

	// 初期化
	void Initialize(Player* player);

	// 更新
	void Update();

	// 描画
	void Draw(KamataEngine::Camera& camera);

	// 敵データの読み込み
	void LoadEnemyData();

	// 外部参照用（GameSceneでの当たり判定などで使用）
	const std::vector<Enemy*>& GetEnemies() const { return enemies_; }
	const std::vector<EnemyAimer*>& GetAimers() const { return aimers_; }
	const std::vector<EnemyHoming*>& GetHomings() const { return homings_; }

private:
	Player* player_ = nullptr;

	// モデル（Managerが一括で管理して使い回す）
	KamataEngine::Model* enemyModel_ = nullptr;
	KamataEngine::Model* enemyAimerModel_ = nullptr;
	KamataEngine::Model* enemyHomingModel_ = nullptr;

	// 弾モデル
	KamataEngine::Model* normalBulletModel_ = nullptr;
	KamataEngine::Model* homingBulletModel_ = nullptr;

	// アクティブな敵リスト
	std::vector<Enemy*> enemies_;
	std::vector<EnemyAimer*> aimers_;
	std::vector<EnemyHoming*> homings_;

	// スポーン待機リスト
	std::list<EnemySpawnData> spawnList_;
	float timer_ = 0.0f;
};