#pragma once
#include "KamataEngine.h"
#include "MyMath.h"

using namespace KamataEngine;

class Player;
class MapChipField; // 地形参照

class Enemy {
public:
	// 敵タイプ
	enum class Type { Walker, Jumper, Chaser, Flyer };

	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	AABB GetAABB();
	Vector3 GetWorldPosition();
	void OnCollision(const Player* player);

	// AI用セットアップ
	void SetType(Type t) { type_ = t; }
	void SetTarget(Player* p) { target_ = p; }          // 追跡で使用
	void SetMapChipField(MapChipField* f) { map_ = f; } // 地面判定で使用
	void OnDamage(float damage, const Vector3& fromDir); // 追加
	bool IsDead() const { return isDead_; }              // 追加
private:
	// 基本
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

	static inline const float kWalkSpeed = 0.02f;
	Vector3 velocity_ = {};

	static inline const float kWalkMotionTime = 1.0f;
	float walkTimer = 0.0f;

	// 当たり判定
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// AI 状態
	Type type_ = Type::Walker;
	Player* target_ = nullptr;
	MapChipField* map_ = nullptr;
	float hp_ = 30.0f;    // 体力
	bool isDead_ = false; // 撃破フラグ

	// ジャンプ・重力（上が+Y）
	static inline const float kJumpVy = 0.25f;
	static inline const float kGrav = 0.01f;
	static inline const float kFallVMax = 0.25f;
	float aiTimer_ = 0.0f;

	// 追跡
	static inline const float kChaseSpeed = 0.03f;
	static inline const float kChaseRange = 6.0f;

	// 飛行
	float baseY_ = 0.0f;
	static inline const float kFlyAmp = 0.5f;
	static inline const float kFlyHz = 0.5f;

	// --- 滑らか振り向き制御（プレイヤーと同様の仕組み） ---
	float turnFirstRotationY_ = 0.0f;           // 振り向き開始角
	float turnTimer_ = 0.0f;                    // 残り時間
	static inline const float kTimeTurn = 0.3f; // 回転時間（秒）
	int facingDir_ = -1;                        // 右=+1, 左=-1（初期は左）
	static inline const float kFaceEps = 0.0001f;

	// 地形ヘルパ
	bool IsGroundBelow(const Vector3& pos) const; // 足元ブロック？
	bool IsLedgeAhead(int dir) const;             // 足場端？
};