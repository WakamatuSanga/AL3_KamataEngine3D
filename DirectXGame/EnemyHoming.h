#pragma once
#include "EnemyHomingBullet.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

class EnemyHoming {
public:
	void Initialize(KamataEngine::Model* model, Player* player);
	void Update();
	void Draw(KamataEngine::Camera& camera);
	~EnemyHoming();

	void OnCollision() { isDead_ = true; }
	float GetCollisionRadius() const;
	bool IsDead() const { return isDead_; }

	const std::vector<EnemyHomingBullet*>& GetBullets() const { return bullets_; }

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	bool isDead_ = false;

	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyHomingBullet*> bullets_;

	Player* player_ = nullptr;

	float moveSpeedZ_ = -0.15f;
	int shotTimer_ = 0;
	int shotInterval_ = 75;
	float bulletSpeed_ = 0.35f;
	float turnRate_ = 0.10f;

	void FireHomingBullet_();
};