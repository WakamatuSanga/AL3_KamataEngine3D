#include "Enemy.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void Enemy::Initialize(Model* model, float dist, float ux, float uy, float speed) {
	model_ = model;
	dist_ = dist;
	ux_ = ux;
	uy_ = uy;
	speed_ = speed;
	isDead_ = false;

	 // フェードパラメータ
	fadeStartDist_ = 25.0f; // 適宜調整
	fadeEndDist_ = 10.0f;
	alpha_ = 0.3f;

	world_.Initialize();
	world_.scale_ = {0.5f, 0.5f, 0.5f};
	world_.rotation_ = {0.0f, 0.0f, 0.0f};
	world_.translation_ = {0.0f, 0.0f, 0.0f};
	world_.matWorld_ = MakeAffineMatrix(world_.scale_, world_.rotation_, world_.translation_);
	world_.TransferMatrix();
}

void Enemy::Damage(int amount) {
	if (isDead_)
		return;
	hp_ -= amount;
	if (hp_ <= 0) {
		isDead_ = true;
		alpha_ = 0.0f;
		// ここで爆発エフェクトを出す予定ならフラグだけ立てておく
	}
}

void Enemy::Update(const RailCamera& rc, float dt) {
	if (isDead_) {
		return;
	}

	// 手前に進む（奥→手前）
	dist_ -= speed_ * dt;

	// 位置計算（今までどおり）
	float halfW = 5.0f;
	float halfH = 3.0f;
	rc.GetViewPlaneHalfExtents(dist_, halfW, halfH);

	const Vector3& eye = rc.Eye();
	const Vector3& fwd = rc.Fwd();
	const Vector3& right = rc.Right();
	const Vector3& up = rc.Up();

	Vector3 center = eye + fwd * dist_;
	Vector3 offset = right * (ux_ * halfW) + up * (uy_ * halfH);
	world_.translation_ = center + offset;

	world_.matWorld_ = MakeAffineMatrix(world_.scale_, world_.rotation_, world_.translation_);
	world_.TransferMatrix();

	// ===== ここから プレイヤー基準のフェード＆消滅 =====

	// プレイヤー平面の距離（GameScene で SetViewPlaneDist(8.0f) に合わせる）
	const float playerDist = 8.0f;
	// プレイヤーより「これだけ手前（カメラ寄り）」までは半透明で残す
	const float fadeBackRange = 5.0f; // 例：2.0f → プレイヤーより2m手前まで半透明

	if (dist_ >= playerDist) {
		// プレイヤーより奥：不透明
		alpha_ = 1.0f;
	} else if (dist_ >= playerDist - fadeBackRange) {
		// プレイヤーとカメラの間：奥→手前へ移動するにつれ半透明にしていく
		// dist_ = playerDist      → alpha = 1.0
		// dist_ = playerDist-Δ    → alpha = 0.3
		float t = (dist_ - (playerDist - fadeBackRange)) / fadeBackRange; // 0〜1
		t = std::clamp(t, 0.0f, 1.0f);
		alpha_ = 0.3f + 0.7f * t; // 0.3〜1.0 に補間
	} else {
		// プレイヤーよりかなり手前（カメラ寄り）まで来たら消す
		alpha_ = 0.0f;
		isDead_ = true;
		return;
	}
}


void Enemy::Draw(Camera& camera) {
	if (isDead_ || !model_) {
		return;
	}
	model_->SetAlpha(alpha_);

	model_->Draw(world_, camera);
}
