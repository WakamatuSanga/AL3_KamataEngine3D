#pragma once

#include "KamataEngine.h"

using namespace KamataEngine;

// 02_09 スライド4枚目
class Enemy {

public:
	// 02_09 スライド5枚目
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	// 02_09 スライド5枚目
	void Update();
	// 02_09 スライド5枚目
	void Draw();

private:
	// 02_09 6枚目 ザ・ワールド
	WorldTransform worldTransform_;
	// 02_09 6枚目 モデル
	Model* model_ = nullptr;
	// 02_09 6枚目 カメラ
	Camera* camera_ = nullptr;

	// 02_09 15枚目
	static inline const float kWalkSpeed = 0.02f;
	// 02_09 15枚目
	Vector3 velocity_ = {};

	// 02_09 19枚目
	static inline const float kWalkMotionAngleStart = 0.0f;
	// 02_09 19枚目
	static inline const float kWalkMotionAngleEnd = 30.0f;
	// 02_09 19枚目
	static inline const float kWalkMotionTime = 1.0f;
	// 02_09 20枚目
	float walkTimer = 0.0f;
};