#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include "Player.h"
#include <algorithm>
#include <cmath>
class EnemyHomingBullet {
public:
	// speed: 弾速（等速） / turnRate: 1フレームの追尾割合 [0..1]
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& spawnPos, float speed, float turnRate, const Player* player);

	void Update();
	void Draw(KamataEngine::Camera& cam);

	bool IsDead() const { return isDead_; }
	// ホーミング固定の設定（fadeStart >= lockDist 推奨）
	void SetHomingLock(float fadeStartDist, float lockDist) {
		fadeStartDist_ = fadeStartDist;
		lockDist_ = lockDist;
	}
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	void OnCollision() { isDead_ = true; }
	float GetCollisionRadius() const;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 velocity_{0, 0, 0};
	float speed_ = 0.1f;
	float turn_ = 0.01f; // 追尾割合（Slerp の t）
	int life_ = 360;     // 寿命フレーム
	bool isDead_ = false;

	// ホーミング固定ロジック用
	bool homingActive_ = true;    // true: 誘導中 / false: 固定直進
	float fadeStartDist_ = 12.0f; // この距離以下で徐々に turn を弱める
	float lockDist_ = 6.0f;       // この距離以下で完全にホーミング停止（直進固定）

	const Player* player_ = nullptr;

	void FaceToVelocity_(); // 見た目を進行方向に
};
