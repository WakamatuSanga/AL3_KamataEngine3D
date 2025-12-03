#pragma once
#include "KamataEngine.h"
#include "PlayerBullet.h"
#include "MyMath.h"
#include <vector>

class Player {
public:
	void Initialize(KamataEngine::Model* model);
	void Update();
	void Draw(KamataEngine::Camera& camera);

	KamataEngine::Vector3 GetPosition() const {
		using namespace KamataEngine;
		Vector3 pos{
		    worldTransform_.matWorld_.m[3][0],
		    worldTransform_.matWorld_.m[3][1],
		    worldTransform_.matWorld_.m[3][2],
		};
		return pos;
	}

	// 前方単位ベクトル（+Z 基準・親の回転も含める）
	KamataEngine::Vector3 GetForwardDir() const {
		using namespace KamataEngine;
		// ワールド行列の 3x3 部分で (0,0,1) を回す
		Vector3 fwd = TransformNormal({0, 0, 1}, worldTransform_.matWorld_);
		return Normalized(fwd);
	}
	// 当たり時コールバック（今回何もしない）
	void OnCollision() {}

	// 自弾リストの貸出し
	const std::vector<PlayerBullet*>& GetBullets() const { return bullets_; }
	~Player();
	float GetCollisionRadius() const;
	void SetParent(const KamataEngine::WorldTransform* parent);
	void SetLocalPosition(const KamataEngine::Vector3& local) { worldTransform_.translation_ = local; }
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
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
