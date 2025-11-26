#pragma once
#include "KamataEngine.h"
#include "MyMath.h" // AABB 用

class EnemyBullet {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	void Update();
	void Draw(KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	// 当たり時は消える
	void OnCollision() { isDead_ = true; }
	float GetCollisionRadius() const;

	// ワールド座標取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// 追加：見た目と寿命の調整
	void SetScale(float s) { worldTransform_.scale_ = {s, s, s}; }
	void SetLifeTimeFrames(int frames) { lifeTime_ = frames; }

	// 既存：進行方向に向ける
	void SetAlignToVelocity(bool enable) { alignToVelocity_ = enable; }

	// 追加：当たり判定（中心＋等方AABB）
	AABB GetAABB() const {
		const float r = collisionRadius_ * worldTransform_.scale_.x;
		const auto& p = worldTransform_.translation_;
		return {
		    {p.x - r, p.y - r, p.z - r},
            {p.x + r, p.y + r, p.z + r}
        };
	}

	void SetLifeTime(int frames) { lifeTime_ = frames; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Vector3 velocity_{};

	int lifeTimer_ = 0;
	int lifeTime_ = 120; // だいたい2秒
	bool isDead_ = false;
	bool alignToVelocity_ = false;

	// 当たり判定半径（モデルに合わせて微調整）
	float collisionRadius_ = 0.5f;

	void FaceToVelocity_();
	void KillIfOutOfRange_();
};
