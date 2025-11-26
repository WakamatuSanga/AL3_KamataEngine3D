#pragma once
#include "KamataEngine.h"
#include "PlayerBullet.h"
#include <vector>

class Player {
public:
	void Initialize(KamataEngine::Model* model);
	void Update();
	void Draw(KamataEngine::Camera& camera);

	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	// 前方単位ベクトル（+Z 基準）
	KamataEngine::Vector3 GetForwardDir() const {
		using namespace KamataEngine;
		Matrix4x4 rx = MakeRotateXMatrix(worldTransform_.rotation_.x);
		Matrix4x4 ry = MakeRotateYMatrix(worldTransform_.rotation_.y);
		Matrix4x4 rz = MakeRotateZMatrix(worldTransform_.rotation_.z);
		Matrix4x4 r = rz * rx * ry;
		Vector3 fwd = TransformNormal({0, 0, 1}, r);
		return Normalized(fwd);
	}
	// 当たり時コールバック（今回何もしない）
	void OnCollision() {}

	// 自弾リストの貸出し
	const std::vector<PlayerBullet*>& GetBullets() const { return bullets_; }
	~Player();
	float GetCollisionRadius() const;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Input* input_ = nullptr;

	// 弾用
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<PlayerBullet*> bullets_;

	// マウス長押し連射制御
	bool mouseHeldPrev_ = false;
	int holdFrames_ = 0;
	int autoFireDelayFrames_ = 20;   // 長押し開始から何フレーム後に連射開始するか
	int autoFireIntervalFrames_ = 5; // 連射間隔
	int autoFireCounter_ = 0;        // ★ 追加：連射カウンタ（static廃止）

	void SpawnBullet();
};
