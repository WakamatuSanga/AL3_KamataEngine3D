#pragma once
#include "KamataEngine.h"
#include "MyMath.h" // AABB 用
#include <cmath>

class PlayerBullet {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	void Update();
	void Draw(KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }
	// 当たり時は消える
	void OnCollision() {
		isDead_ = true;
		lifeTimer_ = lifeTime_; // 念のため寿命も切っておく
	}
	float GetCollisionRadius() const;

	// ワールド座標取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	// 追加API：見た目・寿命・向き
	void SetScale(float s) { worldTransform_.scale_ = {s, s, s}; }
	void SetLifeTimeFrames(int frames) { lifeTime_ = frames; }
	void SetAlignToVelocity(bool enable) { alignToVelocity_ = enable; }

	// 当たり判定用：AABB を返す（中心は現在位置、半径は scale から推定）
	AABB GetAABB() const {
		const float rx = worldTransform_.scale_.x * radius_;
		const float ry = worldTransform_.scale_.y * radius_;
		const float rz = worldTransform_.scale_.z * radius_;
		const auto& p = worldTransform_.translation_;
		return {
		    {p.x - rx, p.y - ry, p.z - rz},
            {p.x + rx, p.y + ry, p.z + rz}
        };
	}

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Vector3 velocity_{};

	int lifeTimer_ = 0; // 生存フレーム数
	int lifeTime_ = 60; // 60フレームで消える
	bool isDead_ = false;

	bool alignToVelocity_ = false; // 進行方向へ向けるか
	float radius_ = 1.0f;          // AABB の半径スケール（見た目に合わせて調整可）

	void FaceToVelocity_(); // 向き更新（alignToVelocity_ 時のみ）
};
