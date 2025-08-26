#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "skydome.h"
#include "Item.h"
#include <vector>
#include <list>
#include <random>
#include <algorithm>
class GameScene {
public:
	~GameScene();
	void Initialize();
	void Update();
	void Draw();
	void GenerateBlocks();
	void CheckAllCollisions();
	bool IsFinished() const { return finished_; }
	bool IsCleared() const { return cleared_; }
	// ChangeScene() から参照する
	bool IsReturnToTitle() const { return requestReturnToTitle_; }

private:
	enum class Phase { kFadeIn, kPlay, kDeath, kFadeOut, kClearFadeOut };
	Phase phase_;
	void ChangePhase();

	// ★ 敵管理
	void ClearEnemies();
	void SpawnRandomEnemies(int total);

	uint32_t textureHandle_ = 0;
	KamataEngine::Sprite* sprite_ = nullptr;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera camera_;
	uint32_t soundDataHandle_ = 0;
	uint32_t voiceHandle_ = 0;

	Player* player_ = nullptr;
	KamataEngine::Model* player_model_ = nullptr;

	KamataEngine::Model* block_model_ = nullptr;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Skydome* skydome_ = nullptr;
	Model* modelSkydome_ = nullptr;

	MapChipField* mapChipField_;

	CameraController* CController_ = nullptr;

	KamataEngine::Model* enemy_model_ = nullptr;
	std::list<Enemy*> enemies_;

	DeathParticles* deathParticles_ = nullptr;
	Model* deathParticle_model_ = nullptr;

	bool finished_ = false;
	Fade* fade_ = nullptr;

	// ▼▼▼ HPバー（UI）▼▼▼
	KamataEngine::Sprite* hpBg_ = nullptr; // 背景
	KamataEngine::Sprite* hpFg_ = nullptr; // 本体（残量）
	                                       // ▲▲▲ HPバー（UI）▲▲▲
	// ====== リスポーン関連 ======
	struct RespawnTicket {
		float timer;
		float yWorld;
	};
	std::vector<RespawnTicket> respawnQueue_;
	std::mt19937 rng_; // 乱数エンジン

	// 復活までの時間[秒]（範囲）
	static inline const float kRespawnDelayMin_ = 3.0f;
	static inline const float kRespawnDelayMax_ = 7.0f;

	// ★ 追加：プレイヤーからの安全マージン（ワールド座標単位、1ブロック=1.0f）
	static inline const float kRespawnMinDistance_ = 15.0f; // 例：8ブロック以上離す
	static inline const int kSpawnSearchTries_ = 40;       // 候補探索の試行回数


	void UpdateRespawns(float dt); // タイマー消化して必要数スポーン
	void EnqueueRespawn();         // 1体分を予約（内部で乱数ディレイ）
	void EnqueueRespawnAt(float yWorld);
	void SpawnEnemyAtSameY(float yWorld);
	void SpawnEnemyRandomVertical();
	void SpawnEnemyAtIndex(uint32_t xIndex, uint32_t yIndex);

	// ====== アイテム関連 ======
	std::vector<Item*> items_;
	Model* item_model_ = nullptr;

	// 配置パラメータ
	static inline const int kItemCount_ = 8;        // 置きたい個数
	static inline const float kItemMinDist_ = 3.0f; // アイテム同士の最小距離（ワールド）
	// 既にある rng_ を流用（なければ std::mt19937 rng_; を用意）

	void SpawnItemsRandom(int count);
	void ClearItems();
	void UpdateItems();
	bool AreAllItemsCollected() const;

	// ====== クリア判定 ======
	bool cleared_ = false;

	Model* attack_model_ = nullptr;
	std::list<DeathParticles*> hitPfx_; // ★追加
	void SpawnHitEffect(const Vector3& pos);

	 // ===== ポーズメニュー =====
	bool paused_ = false;
	int pauseIndex_ = 0; // 0:Resume, 1:Title

	// ポーズUI用スプライト
	KamataEngine::Sprite* pauseDim_ = nullptr;  // 画面暗転
	KamataEngine::Sprite* pauseBtnA_ = nullptr; // Resume
	KamataEngine::Sprite* pauseBtnB_ = nullptr; // Title

	// タイトルに戻る要求（ChangeSceneで分岐するため）
	bool requestReturnToTitle_ = false;
};