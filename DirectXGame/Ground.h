#pragma once
#include "KamataEngine.h"

class Ground {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model);

	// 更新：回転速度（Z軸方向の移動速度に相当する回転量）を受け取る
	void Update(float rotationSpeedX);

	// 描画
	void Draw(KamataEngine::Camera& camera);

	// 位置セット（上下左右スライド用）
	void SetPosition(const KamataEngine::Vector3& pos);

private:
	// 地面（海）のモデル
	KamataEngine::Model* model_ = nullptr;

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// 蓄積回転行列（ボール転がし用）
	KamataEngine::Matrix4x4 matRotationAccumulated_;
};