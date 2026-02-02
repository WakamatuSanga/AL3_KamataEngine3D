#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include <vector>

class Enemy {
public:
	void Initialize(KamataEngine::Model* model);
	void Update();
	void Draw(KamataEngine::Camera& camera);
	~Enemy();

	void OnCollision() { isDead_ = true; }
	float GetCollisionRadius() const;

	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }

	// 向きと速度をセットする関数
	void SetRotation(const KamataEngine::Vector3& rotation) { worldTransform_.rotation_ = rotation; }
	void SetVelocity(const KamataEngine::Vector3& velocity) { approachVelocity_ = velocity; }

	bool IsDead() const { return isDead_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	enum class Phase { Approach, Leave };
	Phase phase_ = Phase::Approach;

	KamataEngine::Vector3 approachVelocity_{};
	KamataEngine::Vector3 leaveVelocity_{};

	bool isDead_ = false;

	void UpdateApproach();
	void UpdateLeave();

	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyBullet*> bullets_;

	int shotTimer_ = 0;
	int shotInterval_ = 45;

	void FireBullet();
	// ★SE
	uint32_t seShoot_ = 0;
};