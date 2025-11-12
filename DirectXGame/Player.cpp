#include "Player.h"
#include "MyMath.h"

using namespace KamataEngine;

void Player::Initialize(Model* model) {
	model_ = model;

	// 自機のワールドトランスフォーム初期化
	world_.Initialize();
	world_.scale_ = {0.7f, 0.7f, 0.7f};
	world_.rotation_ = {0.0f, 0.0f, 0.0f};
	world_.translation_ = {0.0f, 0.0f, 0.0f};
	world_.matWorld_ = MakeAffineMatrix(world_.scale_, world_.rotation_, world_.translation_);
	world_.TransferMatrix();

	// 移動ロジック初期化（WASD & バレルロール）
	move_.Initialize();

	// 弾モデルを用意（sphere が無ければ "cube" に変えてOK）
	bulletModel_ = Model::CreateFromOBJ("item");
	if (!bulletModel_) {
		bulletModel_ = Model::CreateFromOBJ("cube");
	}

	// 射撃ロジック初期化（マウス照準＋左クリック）
	shot_.Initialize(bulletModel_);
}

void Player::Update(const RailCamera& rc) {
	// 固定フレーム（必要ならエンジンのΔtに差し替え）
	const float dt = 1.0f / 60.0f;

	// 画面内XY移動（レールカメラに追従）
	move_.Update(world_, rc);

	// マウス照準＆左クリックで弾を出す
	shot_.Update(world_, rc, dt);
}

void Player::Draw(Camera& cam) {
	// 自機本体
	if (model_) {
		model_->Draw(world_, cam);
	}

	// 弾
	shot_.Draw(cam);
}

// 画面内の位置調整（GameScene から呼ばれているもの）
void Player::SetViewPlaneDist(float d) { move_.SetViewPlaneDist(d); }

void Player::SetScreenBiasY(float frac) { move_.SetScreenBiasY(frac); }
