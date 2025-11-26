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
	// 当たり時（今回は何もしない）
	void OnCollision() {}
	float GetCollisionRadius() const;
	// 敵弾リストの貸出し
	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }

	// あると便利：自分の座標
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	enum class Phase { Approach, Leave };
	Phase phase_ = Phase::Approach;

	KamataEngine::Vector3 approachVelocity_{};
	KamataEngine::Vector3 leaveVelocity_{};

	void UpdateApproach();
	void UpdateLeave();
	void Respawn();

	// ★ ここから弾関連 --------------------
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyBullet*> bullets_;

	int shotTimer_ = 0;
	int shotInterval_ = 45; // 45フレームごとに発射(0.75秒間隔)

	void FireBullet();
};
