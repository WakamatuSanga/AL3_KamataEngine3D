#pragma once
#include "Bullet.h"
#include "KamataEngine.h"
#include "RailCamera.h"
#include <vector>

class PlayerShot {
public:
	void Initialize(KamataEngine::Model* bulletModel);

	// プレイヤーWTとRailCameraから照準＆発射を更新
	void Update(const KamataEngine::WorldTransform& playerWT, const RailCamera& rc, float dt);

	// 自分＋弾の描画
	void Draw(KamataEngine::Camera& camera);

	const KamataEngine::Vector3& GetReticleWorld() const { return reticleWorld_; }

	// GameScene から弾リストを読むため
	const std::vector<Bullet*>& GetBullets() const { return bullets_; }
	void KillBullet(Bullet* b) {
		if (b)
			b->Kill();
	}

private:
	// 発射モード
	enum class FireMode {
		Cursor,  // カーソル追従モード（今までのやつ）
		Straight // 真っ直ぐモード（プレイヤーから前方向固定）
	};
	FireMode mode_ = FireMode::Cursor;

	// 入力
	bool IsMousePressed() const; // 左クリック押した瞬間
	bool IsMouseHeld() const;    // 左クリック押しっぱ

	// 照準：カメラ前方の平面にレティクルを置く
	void UpdateReticleWorld(const RailCamera& rc);

	// カーソルモード用：前方向を中心にしたコーン内の方向
	KamataEngine::Vector3 ComputeAutoDir(const KamataEngine::WorldTransform& playerWT, const RailCamera& rc);

	// 弾生成
	void SpawnBulletStraight(const KamataEngine::WorldTransform& playerWT,
	                         const RailCamera& rc); // 完全に前方向
	void SpawnBulletAuto(const KamataEngine::WorldTransform& playerWT,
	                     const RailCamera& rc); // カーソルモードの連射用

private:
	KamataEngine::Model* bulletModel_ = nullptr;
	std::vector<Bullet*> bullets_;

	// 照準平面
	float aimDist_ = 10.0f;     // カメラから前方の距離
	float mouseSens_ = 0.0045f; // マウス感度
	float margin_ = 0.95f;      // 画角内に収める割合
	float biasYFrac_ = -0.35f;  // 画面下寄せ (-1..+1)

	KamataEngine::Vector2 aimOffset_{0, 0};
	KamataEngine::Vector3 reticleWorld_{0, 0, 10};

	// 発射制御
	float bulletSpeed_ = 40.0f;      // 弾速
	float autoFireDelay_ = 0.25f;    // 長押ししてから連射開始までの時間
	float autoFireInterval_ = 0.08f; // 連射間隔
	float holdTime_ = 0.0f;
	float autoFireTimer_ = 0.0f;
	bool wasHeld_ = false;

	// カーソルモード用：前方向から何度までズラして良いか（コーンの開き）
	float minDot_ = 0.7f; // cosθ (0.7 ≒ 45°)
};
