#pragma once
#include "KamataEngine.h"
#include "RailCamera.h"

class Enemy {
public:
	// dist : カメラからの初期距離
	// ux, uy : 画面内の相対位置 (-1.0f〜+1.0f くらい)
	// speed : 手前方向への速度（正の値）
	void Initialize(KamataEngine::Model* model, float dist, float ux, float uy, float speed);

	// カメラ(レールカメラ)に対して位置を更新
	void Update(const RailCamera& rc, float dt);

	void Draw(KamataEngine::Camera& camera);

	bool IsDead() const { return isDead_; }

	// 当たり判定用
	const KamataEngine::Vector3& GetPosition() const { return world_.translation_; }
	float GetRadius() const { return radius_; }
	void Damage(int amount);

private:
	KamataEngine::WorldTransform world_;
	KamataEngine::Model* model_ = nullptr;

	float dist_ = 0.0f;  // カメラからの距離
	float speed_ = 0.0f; // 手前に向かう速度
	float ux_ = 0.0f;    // 画面内X相対位置 (-1〜+1)
	float uy_ = 0.0f;    // 画面内Y相対位置 (-1〜+1)

	bool isDead_ = false;

	 // フェード制御用
	float alpha_ = 1.0f;          // 0.0f〜1.0f
	float fadeStartDist_ = 25.0f; // ここから薄くなり始める
	float fadeEndDist_ = 10.0f;   // ここまで来たらほぼ見えない＆消す
	// ★ 追加：当たり判定＆HP
    float radius_ = 1.0f; // 見た目より少し小さめに調整
    int hp_ = 1;
};
