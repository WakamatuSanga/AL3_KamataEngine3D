#include "Clouds.h"
#include "MyMath.h"
#include <cmath>   // atan2
#include <cstdlib> // rand()
#include <numbers> // std::numbers::pi_v<float> が使える場合があるが、今回は手打ちで

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

		// Z位置を上書き
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

		// --- 2. ビルボード処理（常にカメラの方を向く） ---
		Vector3 diff = cameraPosition - cloud->worldTransform.translation_;

		// Y軸の回転角度を計算
		// ★修正ポイント: モデルが裏返らないように PI (180度) を足してみる
		// もしこれでまだ消える場合は、 + PI の部分を消してみてください
		cloud->worldTransform.rotation_.y = std::atan2(diff.x, diff.z) + PI;

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

	for (const auto* cloud : clouds_) {
		model_->Draw(cloud->worldTransform, camera);
	}
}

void Clouds::RespawnCloud(CloudData* cloud, const Vector3& basePos) {
	float posX = GetRandom(-spawnRangeX_, spawnRangeX_);
	float posY = GetRandom(-spawnRangeY_, spawnRangeY_);
	float posZ = basePos.z + GetRandom(spawnDistanceMin_, spawnDistanceMax_);

	cloud->worldTransform.translation_ = {posX, posY, posZ};

	// ★ 速度調整
	// プレイヤーを「横切る」感じを出すため、かなり速くしてみます
	// -1.0f ～ -3.0f くらいにアップ
	float speed = GetRandom(1.0f, 3.0f);
	cloud->velocity = {0.0f, 0.0f, -speed};

	float scale = GetRandom(3.0f, 8.0f);
	cloud->worldTransform.scale_ = {scale, scale, scale};

	cloud->worldTransform.rotation_ = {0.0f, 0.0f, 0.0f};
}

float Clouds::GetRandom(float minVal, float maxVal) {
	float r = (float)rand() / RAND_MAX;
	return minVal + r * (maxVal - minVal);
}