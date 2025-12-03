#pragma once
#include "KamataEngine.h" // Camera / WorldTransform / Vector3
#include "MyMath.h"       // MakeAffineMatrix / Inverse

/// レールカメラ（見えない“カメラオブジェクト”をレール上に動かす想定）
class RailCameraController {
public:
	// pos/rot はワールド座標(ラジアン)。nearZ/farZ/fovY は必要に応じて調整
	void Initialize(const KamataEngine::Vector3& pos = {0, 2.0f, -10.0f}, const KamataEngine::Vector3& rot = {0, 0, 0}, float fovY = 0.45f, float nearZ = 0.1f, float farZ = 500.0f);

	void Update();   // 今は入力なしの素通し（必要ならここで移動/回転）
	void DebugGui(); // 任意：ImGui スライダ

	// 描画に使うのはこの Camera の行列
	KamataEngine::Camera& GetCamera() { return camera_; }
	const KamataEngine::Camera& GetCamera() const { return camera_; }
	// 親子付けのため：カメラ“オブジェクト”のワールド変換
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// パラメータ変更
	void SetPerspective(float fovY, float nearZ, float farZ) {
		camera_.fovAngleY = fovY;
		camera_.nearZ = nearZ;
		camera_.farZ = farZ;
	}
	void SetFarZ(float farZ) { camera_.farZ = farZ; }

	// ▼ デバッグ自動回転用パラメータ（公開にしてImGuiでいじれるように）
	bool dbgSpin_ = false;                    // 自動回転のオン/オフ
	float dbgDegPerSec_ = 10.0f;              // 回転速度(度/秒)
	float dbgRadius_ = 40.0f;                 // 回転半径
	float dbgHeight_ = 8.0f;                  // 目線の高さ
	KamataEngine::Vector3 dbgPivot_{0, 0, 0}; // 回転の中心(注視点)
private:
	KamataEngine::WorldTransform worldTransform_{};
	KamataEngine::Camera camera_{};
	KamataEngine::Input* input_ = nullptr;
	float dbgAngle_ = 0.0f;
};
