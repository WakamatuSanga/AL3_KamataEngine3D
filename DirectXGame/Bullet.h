#pragma once
#include "KamataEngine.h"

class Bullet {
public:
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	void Update(float dt);
	void Draw(KamataEngine::Camera& camera);

	bool IsDead() const { return life_ <= 0.0f; }

	const KamataEngine::Vector3& GetPosition() const { return world_.translation_; }
	float GetRadius() const { return radius_; }
	void Kill() { life_ = 0.0f; }

private:
	KamataEngine::WorldTransform world_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Vector3 velocity_{};
	float life_ = 0.0f; // 寿命(秒)

	// 当たり判定用半径
	float radius_ = 0.4f;
};
