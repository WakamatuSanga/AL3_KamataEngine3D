#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

// 画面（カメラ）に追従してとどまる敵
class EnemyFollow {
public:
	void Initialize(KamataEngine::Model* model, Player* player, const KamataEngine::Vector3& offset);

	// カメラの行列を受け取って位置を計算する
	void Update(const KamataEngine::Matrix4x4& cameraMat);

	void Draw(KamataEngine::Camera& camera);
	~EnemyFollow();

	void OnCollision() { isDead_ = true; }
	float GetCollisionRadius() const;
	bool IsDead() const { return isDead_; }

	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	bool isDead_ = false;

	// 弾
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyBullet*> bullets_;
	Player* player_ = nullptr;

	// カメラからの相対位置（オフセット）
	KamataEngine::Vector3 offset_ = {0, 0, 0};

	// 制御用
	int lifeTimer_ = 0;
	int lifeTime_ = 600; // 10秒くらい画面にとどまる

	int shotTimer_ = 0;
	int shotInterval_ = 90; // 1.5秒に1回発射

	// 出現演出用
	float appearTimer_ = 0.0f;

	void FireAimedBullet();

	// ★SE
	uint32_t seShoot_ = 0;
};