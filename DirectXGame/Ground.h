#pragma once
#include "KamataEngine.h"
#include <memory>
#include <vector>

class Ground {
public:
	// ① “板を薄く伸ばす”方式（OBJなしでもOK）
	//    width: タイルの横幅（Xスケール）
	//    segLen: タイルの奥行長（Z方向の長さ）
	//    count: 何枚つなげるか
	//    y: 地面の高さ
	//    speed: 手前へのスクロール速度（Z-方向へ移動）
	void Initialize(KamataEngine::Model* model, float width = 80.0f, float segLen = 20.0f, int count = 16, float y = -6.0f, float speed = 0.6f);

	// ② OBJの凹凸形状をそのまま使う方式（推奨）
	//    stepZ: タイル間の奥行き間隔（OBJ1枚の奥行に合わせる）
	//    uniformScale: OBJの等倍スケール（全体の大きさ調整）
	void InitializeOBJ(KamataEngine::Model* model, float stepZ = 20.0f, int count = 16, float y = -6.0f, float speed = 0.6f, float uniformScale = 1.0f);

	void Update();
	void Draw(KamataEngine::Camera& cam);

	// 調整用
	void SetSpeed(float s) { scrollSpeed_ = s; }
	void SetY(float y);
	void SetParallaxX(float k) { parallaxX_ = k; }         // 視差（任意、0で無効）
	void SetPlayerX(float playerX) { playerX_ = playerX; } // 視差の基準

	 void StartAtRecycleLine(float cameraZ);

	// OBJ方式のスケール変更（必要なら）
	void SetUniformScale(float s) { objUniformScale_ = s; }

	// デフォは「-segLen_ で再配置」。そこからさらに奥へ下げたい量（+m）
	void SetRecycleBackExtra(float extra) { recycleBackExtra_ = extra; }

	// カメラより backMore だけ“さらに後ろ”まで行ってから再配置
	void UseBehindCameraRecycle(float backMore) {
		recycleBehindCamera_ = true;
		recycleBehindMore_ = backMore;
	}
	void DisableBehindCameraRecycle() { recycleBehindCamera_ = false; }

	// 毎フレーム、カメラのワールドZを渡す（UseBehindCameraRecycle使用時）
	void SetCameraZ(float z) { cameraZ_ = z; }

private:
	KamataEngine::Model* model_ = nullptr;

	// タイル（各セグメントのワールド変換）
	std::vector<std::unique_ptr<KamataEngine::WorldTransform>> tiles_;

	// タイル列のパラメータ
	float width_ = 80.0f;      // 板方式のXスケール
	float segLen_ = 20.0f;     // 1タイルの奥行長（共通）
	int count_ = 16;           // タイル枚数
	float y_ = -6.0f;          // 高さ
	float scrollSpeed_ = 0.6f; // 手前方向（Z-）へのスクロール速度

	// 視差（任意）: プレイヤーのXに応じて地面をわずかに動かす
	float playerX_ = 0.0f;
	float parallaxX_ = 0.2f;

	// OBJ形状そのまま描画フラグ
	bool objMode_ = false;
	float objUniformScale_ = 1.0f;

	   float recycleBackExtra_ = 0.0f; // 既定(-segLen_)からさらに後ろへ
	bool recycleBehindCamera_ = false;
	float cameraZ_ = 0.0f;
	float recycleBehindMore_ = 20.0f; // カメラよりさらにどれだけ後ろで再配置するか
};
