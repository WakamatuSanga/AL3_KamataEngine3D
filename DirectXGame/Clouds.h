#pragma once
#include "KamataEngine.h"
#include <vector>

// 個々の雲のデータを管理する構造体
struct CloudData {
	KamataEngine::WorldTransform worldTransform;
	KamataEngine::Vector3 velocity;
	bool isActive;
};

class Clouds {
public:
	// デストラクタを追加（メモリ解放のため）
	~Clouds();

	// numClouds: 雲の数
	void Initialize(KamataEngine::Model* model, int numClouds = 20);

	// カメラの位置を受け取って更新する
	void Update(const KamataEngine::Vector3& cameraPosition);

	void Draw(KamataEngine::Camera& camera);

private:
	KamataEngine::Model* model_ = nullptr;

	// ★変更: 実体(CloudData)ではなく、ポインタ(CloudData*)を保存するように変更
	// これにより WorldTransform のコピー禁止エラーを回避します
	std::vector<CloudData*> clouds_;

	// 雲が出現する範囲の設定
	float spawnRangeX_ = 60.0f;
	float spawnRangeY_ = 40.0f;

	// カメラからどれくらい奥に出現させるか
	float spawnDistanceMin_ = 200.0f;
	float spawnDistanceMax_ = 400.0f;

	// カメラからどれくらい後ろに行ったら消す（再配置する）か
	float disappearZ_ = -20.0f;

	// 乱数生成器
	float GetRandom(float minVal, float maxVal);

	// 雲をひとつ再配置する関数（引数をポインタに変更）
	void RespawnCloud(CloudData* cloud, const KamataEngine::Vector3& basePos);
};