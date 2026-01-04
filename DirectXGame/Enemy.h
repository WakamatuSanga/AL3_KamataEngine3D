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
	
	// 当たり判定用
	void OnCollision() { isDead_ = true; } // 当たったら死亡
	float GetCollisionRadius() const;

	// 敵弾リストの貸出し
	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }

	// 座標取得・設定
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }

	// 死亡フラグ
	bool IsDead() const { return isDead_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	// 動作フェーズ
	enum class Phase { Approach, Leave };
	Phase phase_ = Phase::Approach;

	KamataEngine::Vector3 approachVelocity_{};
	KamataEngine::Vector3 leaveVelocity_{};
	
	bool isDead_ = false; // 死亡フラグ

	void UpdateApproach();
	void UpdateLeave();
	
	// 弾関連
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyBullet*> bullets_;

	int shotTimer_ = 0;
	int shotInterval_ = 45;

	void FireBullet();
};