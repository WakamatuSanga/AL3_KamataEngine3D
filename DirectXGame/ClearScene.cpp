#include "ClearScene.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "MyMath.h"
#include <cmath>

using namespace KamataEngine;

void ClearScene::Initialize() {
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -15.0f};
	camera_.UpdateMatrix();

	// "clear_text" モデルなどを読み込む
	modelText_ = Model::CreateFromOBJ("clear_text");
	if (!modelText_)
		modelText_ = Model::Create(); // フォールバック

	wtText_.Initialize();
	wtText_.translation_ = {0.0f, 3.0f, 0.0f};
	wtText_.rotation_ = {0.0f, 3.14f, 0.0f};
	wtText_.scale_ = {1.5f, 1.5f, 1.5f};

	// プレイヤーモデル
	modelPlayer_ = Model::CreateFromOBJ("player");
	if (!modelPlayer_)
		modelPlayer_ = Model::Create();

	wtPlayer_.Initialize();
	wtPlayer_.translation_ = {0.0f, -1.0f, 0.0f};
}

void ClearScene::Update() {
	// テキストをバウンドさせる
	static float bounce = 0.0f;
	bounce += 0.1f;
	wtText_.translation_.y = 3.0f + std::abs(std::sin(bounce)) * 0.5f;

	// プレイヤーを高速回転（喜びの表現）
	wtPlayer_.rotation_.y += 0.1f;
	// 上へ飛んでいく演出など
	wtPlayer_.translation_.y += 0.01f;

	// 行列更新
	wtText_.matWorld_ = MakeAffineMatrix(wtText_.scale_, wtText_.rotation_, wtText_.translation_);
	wtText_.TransferMatrix();

	wtPlayer_.matWorld_ = MakeAffineMatrix(wtPlayer_.scale_, wtPlayer_.rotation_, wtPlayer_.translation_);
	wtPlayer_.TransferMatrix();

	// スペースキーでタイトルへ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(new TitleScene());
	}
}

void ClearScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();

	Model::PreDraw(dxCommon->GetCommandList());
	if (modelText_)
		modelText_->Draw(wtText_, camera_);
	if (modelPlayer_)
		modelPlayer_->Draw(wtPlayer_, camera_);
	Model::PostDraw();

}

ClearScene::~ClearScene() {
	delete modelText_;
	delete modelPlayer_;
}