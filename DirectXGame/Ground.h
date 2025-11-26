#pragma once
#include "KamataEngine.h"
#include <algorithm>
#include <memory>
#include <vector>

class Ground {
public:
	// ── 板方式（従来の薄い箱のタイル）
	void Initialize(KamataEngine::Model* model, float width = 80.0f, float segLen = 20.0f, int countZ = 16, float y = -6.0f, float speed = 0.6f, int columns = 1, float colSpacingX = 8.0f);

	// ── OBJ方式（OBJの凹凸をそのまま使う）
	void InitializeOBJ(KamataEngine::Model* model, float stepZ = 20.0f, int countZ = 16, float y = -6.0f, float speed = 0.6f, float uniformScale = 1.0f, int columns = 1, float colSpacingX = 8.0f);

	void Update();
	void Draw(KamataEngine::Camera& cam);

	// ---- 調整系 ----
	void SetSpeed(float s) { scrollSpeed_ = s; }
	void SetY(float y);
	void SetParallaxX(float k) { parallaxX_ = k; }          // 視差
	void SetPlayerX(float playerX) { playerX_ = playerX; }  // 視差の基準
	void SetUniformScale(float s) { objUniformScale_ = s; } // OBJ方式の等倍スケール
	void SetColumns(int c) { columns_ = max(1, c); }
	void SetColumnSpacing(float s) { colSpacingX_ = s; }

	// ---- 再配置ライン（消える位置）の調整 ----
	void SetRecycleBackExtra(float extra) { recycleBackExtra_ = extra; }
	void UseBehindCameraRecycle(float backMore) {
		recycleBehindCamera_ = true;
		recycleBehindMore_ = backMore;
	}
	void DisableBehindCameraRecycle() { recycleBehindCamera_ = false; }
	void SetCameraZ(float z) { cameraZ_ = z; }

	// ---- 初期並べ位置のヘルパ ----
	void StartAtCameraFront(float cameraZ, int tilesInFront, float margin = 0.0f);
	void StartAtRecycleLine(float cameraZ);

private:
	KamataEngine::Model* model_ = nullptr;
	std::vector<std::unique_ptr<KamataEngine::WorldTransform>> tiles_;

	// Z方向タイル数・各タイルの奥行
	int countZ_ = 16;
	float segLen_ = 20.0f;

	// X方向の列
	int columns_ = 1;
	float colSpacingX_ = 8.0f; // 列の中心間隔

	// 板方式パラメータ
	float width_ = 80.0f;

	// 共通
	float y_ = -6.0f;
	float scrollSpeed_ = 0.6f;

	// 視差
	float playerX_ = 0.0f;
	float parallaxX_ = 0.2f;

	// OBJ方式
	bool objMode_ = false;
	float objUniformScale_ = 1.0f;

	// 再配置ライン調整
	float recycleBackExtra_ = 0.0f;
	bool recycleBehindCamera_ = false;
	float cameraZ_ = 0.0f;
	float recycleBehindMore_ = 20.0f;

	// 列の中心X座標（0基準で左右対称）
	inline float ColumnOffsetX_(int col) const {
		// col ∈ [0, columns_-1], 中心を0に
		const float center = (columns_ - 1) * 0.5f;
		return (col - center) * colSpacingX_;
	}
};
