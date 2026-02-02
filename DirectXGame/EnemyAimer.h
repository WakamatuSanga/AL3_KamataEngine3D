#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

class EnemyAimer {
public:
	void Initialize(KamataEngine::Model* model, Player* player);
	void Update();
	void Draw(KamataEngine::Camera& camera);
	~EnemyAimer();

	void OnCollision() { isDead_ = true; }
	float GetCollisionRadius() const;
	bool IsDead() const { return isDead_; }

	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }

	void SetRotation(const KamataEngine::Vector3& rotation) { worldTransform_.rotation_ = rotation; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	bool isDead_ = false;

	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyBullet*> bullets_;

	Player* player_ = nullptr;

	float moveSpeedZ_ = -0.2f;
	int shotTimer_ = 0;
	int shotInterval_ = 60;
	float stopShootMarginZ_ = 0.0f;

	void FireAimedBullet();
	// ★SE
	uint32_t seShoot_ = 0;
};