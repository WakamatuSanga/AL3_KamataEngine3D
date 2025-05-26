#include "KamataEngine.h"
class Player;
using namespace KamataEngine;
using namespace MathUtility;

class CameraController {
public:
	// 短形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};
	Rect movableArea_ = {0, 100, 0, 100};

	// 初期化
	void Initialize();
	// 更新
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	const KamataEngine::Camera& GetViewProjection() const { return camera_; }

	void Reset();

	void SetMovableArea(Rect area) { movableArea_ = area; }

private:
	// カメラ
	Camera camera_;
	Player* target_ = nullptr;

	// 追跡対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = {0, 0, -15.0f};
	// カメラの目標座標
	Vector3 targetCoordinates_;
	// 座標補間割合
	static inline const float kInterpolationRate = 0.05f;
	// 速度掛け率
	static inline const float kVelocityBias = 30;

	static inline const Rect targetMargin = {-9.0f, 9.0f, -5.0f, 5.0f};
};
