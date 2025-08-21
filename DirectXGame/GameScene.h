#pragma once
#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "skydome.h"
#include <vector>

class GameScene {
public:
	~GameScene();
	void Initialize();
	void Update();
	void Draw();
	void GenerateBlocks();
	void CheckAllCollisions();
	bool IsFinished() const { return finished_; }

private:
	enum class Phase { kFadeIn, kPlay, kDeath, kFadeOut };
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
};
