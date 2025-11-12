#pragma once
#include "KamataEngine.h"
#include "PlayerMovement.h"
#include "PlayerShot.h"
#include "RailCamera.h"

class Player {
public:
	// GameScene からプレイヤーモデルを渡してもらう
	void Initialize(KamataEngine::Model* model);

	// レールカメラに追従して移動＆射撃更新
	void Update(const RailCamera& rc);

	// 自機本体＋弾の描画
	void Draw(KamataEngine::Camera& cam);

	// 画面内のどこにプレイヤーを動かすか
	void SetViewPlaneDist(float d);
	void SetScreenBiasY(float frac);

	// GameScene から弾情報を取るためのラッパー
	const std::vector<Bullet*>& GetBullets() const { return shot_.GetBullets(); }
	void KillBullet(Bullet* b) { shot_.KillBullet(b); }

private:
	KamataEngine::WorldTransform world_{};
	KamataEngine::Model* model_ = nullptr; // 機体モデル（GameSceneから渡される）

	PlayerMovement move_; // WASD移動＆バレルロール
	PlayerShot shot_;     // マウス照準＋左クリック射撃

	KamataEngine::Model* bulletModel_ = nullptr; // 弾モデル（sphereやcube）
};
