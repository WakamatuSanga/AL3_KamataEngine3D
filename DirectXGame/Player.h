#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include "PlayerBullet.h"
#include <vector>

// 前方宣言
class EnemyManager;

class Player {
public:
	void Initialize(KamataEngine::Model* model);

	// EnemyManager* を引数に追加
	void Update(const KamataEngine::Camera& camera, const EnemyManager* enemyManager);

	void Draw(KamataEngine::Camera& camera);
	void DrawUI();

	KamataEngine::Vector3 GetPosition() const {
		using namespace KamataEngine;
		Vector3 pos{
		    worldTransform_.matWorld_.m[3][0],
		    worldTransform_.matWorld_.m[3][1],
		    worldTransform_.matWorld_.m[3][2],
		};
		return pos;
	}

	KamataEngine::Vector3 GetForwardDir() const {
		using namespace KamataEngine;
		Vector3 fwd = TransformNormal({0, 0, 1}, worldTransform_.matWorld_);
		return Normalized(fwd);
	}
	void OnCollision();

	bool IsDead() const { return isDead_; }
	int GetHP() const { return hp_; }

	const std::vector<PlayerBullet*>& GetBullets() const { return bullets_; }
	~Player();
	float GetCollisionRadius() const;
	void SetParent(const KamataEngine::WorldTransform* parent);
	void SetLocalPosition(const KamataEngine::Vector3& local) { worldTransform_.translation_ = local; }
	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Input* input_ = nullptr;

	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<PlayerBullet*> bullets_;

	bool mouseHeldPrev_ = false;
	int holdFrames_ = 0;
	int autoFireDelayFrames_ = 20;
	int autoFireIntervalFrames_ = 5;
	int autoFireCounter_ = 0;

	int hp_ = 10;
	float maxHp_ = 10.0f; // ★追加：HPバー計算用の最大値
	bool isDead_ = false;
	int invincibilityTimer_ = 0;
	static const int kInvincibilityTime = 60;

	KamataEngine::Sprite* spriteReticle_ = nullptr;
	KamataEngine::Vector3 target3DPos_ = {0, 0, 0};

	// HPバー用のスプライト
	KamataEngine::Sprite* spriteHPBarBG_ = nullptr; // 背景（減った部分）
	KamataEngine::Sprite* spriteHPBar_ = nullptr;   // 前景（残っている部分）

	// SEハンドル
	uint32_t seShoot_ = 0;
	uint32_t seHit_ = 0;

	void SpawnBullet();
};