#pragma once
#include "KamataEngine.h"
#include <vector>
using namespace KamataEngine;

// ---------------------------
// 弾（単体）
// ---------------------------
struct Bullet {
	Vector3 pos{};
	Vector3 vel{};
	float life = 0.0f;
	float radius = 0.2f;
	bool piercing = false; // チャージ弾なら true
	bool alive = false;

	WorldTransform wt;

	Bullet(); // 明示的デフォルトコンストラクタ
};

// ---------------------------
// 弾マネージャ（プール）
// ---------------------------
class BulletManager {
public:
	void Initialize();
	void SetModels(Model* normal, Model* charged);

	void SpawnNormal(const Vector3& pos, const Vector3& dir);
	void SpawnCharged(const Vector3& pos, const Vector3& dir);

	void Update(float dt);
	void Draw(Camera& cam);

private:
	Bullet* Alloc();

private:
	std::vector<Bullet> pool_;
	Model* modelNormal_ = nullptr;
	Model* modelCharged_ = nullptr;

	// 調整用パラメータ
	float speedNormal_ = 80.0f;
	float speedCharged_ = 100.0f;
	float lifeNormal_ = 2.0f;
	float lifeCharged_ = 3.0f;
	float radiusNormal_ = 0.15f;
	float radiusCharged_ = 0.30f;
};
