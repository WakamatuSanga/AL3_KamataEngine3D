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
	const std::vector<EnemyHomingBullet*>& GetBullets() const { return bullets_; }
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	float GetCollisionRadius() const;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;

	// 弾モデルは内部で生成（Enemy/EnemyAimer と同じ所有ポリシー）
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyHomingBullet*> bullets_;
	
	Player* player_ = nullptr;

	// 移動
	float moveSpeedZ_ = -0.15f;

	// 射撃制御
	int shotTimer_ = 0;
	int shotInterval_ = 75; // 約1.25秒ごと
	float bulletSpeed_ = 0.35f;
	float turnRate_ = 0.10f; // 追尾割合(0..1)

	void FireHomingBullet_();
	void RespawnIfFar_();
};
