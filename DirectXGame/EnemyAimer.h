#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

class EnemyAimer {
public:
	// model … 見た目用モデル
	// player … 狙う対象
	void Initialize(KamataEngine::Model* model, Player* player);
	void Update();
	void Draw(KamataEngine::Camera& camera);
	~EnemyAimer();
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

	// 弾
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<EnemyBullet*> bullets_;

	// プレイヤー参照
	Player* player_ = nullptr;

	// 移動
	float moveSpeedZ_ = -0.2f; // 手前へ

	// 射撃制御
	int shotTimer_ = 0;
	int shotInterval_ = 60;         // 60フレームごとに発射
	float stopShootMarginZ_ = 0.0f; // プレイヤーZ+margin を超えたら発射停止


	void FireAimedBullet();
	void RespawnIfFar();
};
