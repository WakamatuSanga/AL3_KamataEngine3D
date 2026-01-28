#include "Clouds.h"
#include "MyMath.h"
#include <algorithm> // std::sort
#include <cmath>     // atan2
#include <cstdlib>   // rand()

using namespace KamataEngine;

Clouds::~Clouds() {
	for (auto* cloud : clouds_) {
		delete cloud;
	}
	clouds_.clear();
}

void Clouds::Initialize(Model* model, int numClouds) {
	model_ = model;

	// 再初期化に備えて既存のものを削除
	for (auto* cloud : clouds_) {
		delete cloud;
	}
	clouds_.clear();

	// 最初にランダムに配置
	for (int i = 0; i < numClouds; ++i) {
		CloudData* newCloud = new CloudData();
		newCloud->worldTransform.Initialize();

		float startZ = GetRandom(-20.0f, spawnDistanceMax_);
		Vector3 basePos = {0, 0, startZ};

		RespawnCloud(newCloud, basePos);

		// 初期配置だけはZ位置をばらけさせるために上書き
		newCloud->worldTransform.translation_.z = startZ;

		// 行列更新
		newCloud->worldTransform.matWorld_ = MakeAffineMatrix(newCloud->worldTransform.scale_, newCloud->worldTransform.rotation_, newCloud->worldTransform.translation_);
		newCloud->worldTransform.TransferMatrix();

		clouds_.push_back(newCloud);
	}
}

void Clouds::Update(const Vector3& cameraPosition) {
	for (auto* cloud : clouds_) {
		// --- 1. 移動処理 ---
		// ワールド座標系で手前(-Z)へ移動
		cloud->worldTransform.translation_ += cloud->velocity;

		// カメラより「後ろ」に行ったら（＝通り過ぎたら）、前方に再配置
		if (cloud->worldTransform.translation_.z < cameraPosition.z + disappearZ_) {
			RespawnCloud(cloud, cameraPosition);
		}

		// --- 2. ビルボード処理 ---
		Vector3 diff = cameraPosition - cloud->worldTransform.translation_;

		// ★前回の修正を維持（見えた設定）
		// 90度 (約1.57) ずらして正面を向ける
		cloud->worldTransform.rotation_.y = std::atan2(diff.x, diff.z) + 1.5708f;

		cloud->worldTransform.rotation_.x = 0.0f;
		cloud->worldTransform.rotation_.z = 0.0f;

		// 行列更新
		cloud->worldTransform.matWorld_ = MakeAffineMatrix(cloud->worldTransform.scale_, cloud->worldTransform.rotation_, cloud->worldTransform.translation_);
		cloud->worldTransform.TransferMatrix();
	}
}

void Clouds::Draw(Camera& camera) {
	if (!model_)
		return;

	// ★「奥から手前（降順）」の設定
	std::sort(clouds_.begin(), clouds_.end(), [&](CloudData* a, CloudData* b) {
		Vector3 diffA = a->worldTransform.translation_ - camera.translation_;
		Vector3 diffB = b->worldTransform.translation_ - camera.translation_;
		float distSqA = Dot(diffA, diffA);
		float distSqB = Dot(diffB, diffB);

		// 降順（大きい方が先＝奥から描く）
		return distSqA > distSqB;
	});

	// ソートされた順序で描画
	for (const auto* cloud : clouds_) {
		model_->Draw(cloud->worldTransform, camera);
	}
}

void Clouds::RespawnCloud(CloudData* cloud, const Vector3& basePos) {
	// 0:左, 1:右, 2:下
	int side = rand() % 3;

	float posX = 0.0f;
	float posY = 0.0f;
	float scale = 1.0f;

	// 中央のトンネル幅
	const float tunnelWidth = 40.0f;
	// 外側の出現限界
	float outerX = spawnRangeX_ * 1.5f;

	switch (side) {
	case 0: // 左サイド
		posX = GetRandom(-outerX, -tunnelWidth);
		posY = GetRandom(-spawnRangeY_, spawnRangeY_);
		break;

	case 1: // 右サイド
		posX = GetRandom(tunnelWidth, outerX);
		posY = GetRandom(-spawnRangeY_, spawnRangeY_);
		break;

	case 2: // 下サイド（毛嵐）
		posX = GetRandom(-outerX, outerX);
		posY = GetRandom(-spawnRangeY_ * 1.5f, -10.0f);
		break;
	}

	// --- スケール計算（全体的に小さく調整しました） ---
	if (side == 2) {
		// 下の雲（毛嵐）：
		// 前回 30～60 -> 今回 12～25 くらいに縮小
		scale = GetRandom(12.0f, 25.0f);
	} else {
		// 左右の雲：
		float t = (posY + spawnRangeY_) / (spawnRangeY_ * 2.0f);
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;

		float sizeFactor = 1.0f - t;

		// 上空なら 2.0f、下の方なら 12.0f くらいになるように計算
		// 前回は 5.0～25.0 だったので半分以下です
		float baseScale = 2.0f + (sizeFactor * 10.0f);

		scale = baseScale * GetRandom(0.8f, 1.2f);
	}

	float posZ = basePos.z + GetRandom(spawnDistanceMin_, spawnDistanceMax_);
	cloud->worldTransform.translation_ = {posX, posY, posZ};
	cloud->worldTransform.scale_ = {scale, scale, scale};

	// 速度調整
	float speed = GetRandom(1.5f, 3.5f);
	cloud->velocity = {0.0f, 0.0f, -speed};

	cloud->worldTransform.rotation_ = {0.0f, 0.0f, 0.0f};
}

float Clouds::GetRandom(float minVal, float maxVal) {
	float r = (float)rand() / RAND_MAX;
	return minVal + r * (maxVal - minVal);
}