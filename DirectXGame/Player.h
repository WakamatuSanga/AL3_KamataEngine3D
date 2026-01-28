#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include "PlayerBullet.h"
#include <vector>

class Player {
public:
	void Initialize(KamataEngine::Model* model);

	// ★変更：Updateにカメラ情報を渡す（座標計算用）
	void Update(const KamataEngine::Camera& camera);

	void Draw(KamataEngine::Camera& camera);

	// ★追加：2D UI（レティクル）の描画用
	void DrawUI();

	KamataEngine::Vector3 GetPosition() const {
		using namespace KamataEngine;
		Vector3 pos{
		    worldTransform_.matWorld_.m[3][0],
		    worldTransform_.matWorld_.m[3][1],
		    worldTransform_.matWorld_.m[3][2],
		};
		return pos;
	}

	KamataEngine::Vector3 GetForwardDir() const {
		using namespace KamataEngine;
		Vector3 fwd = TransformNormal({0, 0, 1}, worldTransform_.matWorld_);
		return Normalized(fwd);
	}
	void OnCollision();

	bool IsDead() const { return isDead_; }
	int GetHP() const { return hp_; }

	const std::vector<PlayerBullet*>& GetBullets() const { return bullets_; }
	~Player();
	float GetCollisionRadius() const;
	void SetParent(const KamataEngine::WorldTransform* parent);
	void SetLocalPosition(const KamataEngine::Vector3& local) { worldTransform_.translation_ = local; }
	KamataEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Input* input_ = nullptr;

	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<PlayerBullet*> bullets_;

	// 連射制御
	bool mouseHeldPrev_ = false;
	int holdFrames_ = 0;
	int autoFireDelayFrames_ = 20;
	int autoFireIntervalFrames_ = 5;
	int autoFireCounter_ = 0;

	int hp_ = 10;
	bool isDead_ = false;
	int invincibilityTimer_ = 0;
	static const int kInvincibilityTime = 60;

	// ★追加：レティクル制御
	KamataEngine::Sprite* spriteReticle_ = nullptr;
	KamataEngine::Vector3 target3DPos_ = {0, 0, 0}; // 3D空間上の狙っている位置
	float targetDistance_ = 100.0f;                 // 狙う奥行き（カメラからの距離）

	void SpawnBullet();
};