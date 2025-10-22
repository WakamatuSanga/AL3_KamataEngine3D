#pragma once
#include "Bullet.h"
#include "KamataEngine.h"
#include "RailCamera.h"
using namespace KamataEngine;

// マウス照準 + 左クリック：通常 / チャージ
class PlayerShooter {
public:
	void Initialize();
	void SetBulletManager(BulletManager* mgr) { bullets_ = mgr; }
	void SetModels(Model* bulletNormal, Model* bulletCharged);

	// 調整
	void SetAimDistance(float d) { aimDist_ = d; }
	void SetMouseSensitivity(float s) { mouseSens_ = s; }

	// 毎フレーム更新
	void Update(const WorldTransform& playerWT, const RailCamera& rc, float dt);

	// デバッグ/UI用
	const Vector3& ReticleWorld() const { return reticleWorld_; }

private:
	// 入力
	bool IsMousePressed() const; // 押された瞬間
	bool IsMouseHeld() const;    // 押下中（WinAPI補完）

	// マウス相対移動でレティクル更新
	void UpdateReticleWorld(const RailCamera& rc);

private:
	BulletManager* bullets_ = nullptr;
	Model* modelBullet_ = nullptr;
	Model* modelCharged_ = nullptr;

	// 照準（カメラ前方の平面）
	float aimDist_ = 10.0f;
	float mouseSens_ = 0.0045f;             // 相対移動 → 平面座標 係数
	float margin_ = 0.95f;                  // 画角の内側にクランプ
	float biasYFrac_ = -0.35f;              // 下寄せ割合（-1..+1 を半高に掛ける）
	Vector2 aimOffset_{0, 0}; // 平面上のオフセット
	Vector3 reticleWorld_{0, 0, 10};

	// 発射管理
	float rps_ = 10.0f;       // 通常弾レート（発/秒）
	float cd_ = 0.0f;         // クールダウン
	float charge_ = 0.0f;     // チャージ時間
	float chargeNeed_ = 0.6f; // これ以上でチャージ弾
	float chargeMax_ = 1.2f;
	bool heldPrev_ = false;
};
