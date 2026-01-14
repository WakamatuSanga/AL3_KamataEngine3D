#include "TitleScene.h"
#include "GameScene.h"
#include "SceneManager.h"
#include <cmath>

using namespace KamataEngine;

void TitleScene::Initialize() {
	// カメラ初期化
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -20.0f}; // 少し引く
	camera_.UpdateMatrix();

	// タイトル用モデル（無ければ cube で代用）
	modelTitle_ = Model::CreateFromOBJ("title_text");
	if (!modelTitle_)
		modelTitle_ = Model::CreateFromOBJ("cube");
	if (!modelTitle_)
		modelTitle_ = Model::Create(); // 最終フォールバック

	wtTitle_.Initialize();
	wtTitle_.scale_ = {2.0f, 2.0f, 2.0f};
	wtTitle_.translation_ = {0.0f, 2.0f, 0.0f};

	// キャラクターモデル
	modelChar_ = Model::CreateFromOBJ("player");
	if (!modelChar_)
		modelChar_ = Model::Create();

	wtChar_.Initialize();
	wtChar_.translation_ = {0.0f, -2.0f, 0.0f};
}

void TitleScene::Update() {
	// タイトルモデルを回転させる演出
	wtTitle_.rotation_.y += 0.02f;

	// キャラクターをふわふわさせる
	static float timer = 0.0f;
	timer += 0.05f;
	wtChar_.translation_.y = -2.0f + std::sin(timer) * 0.5f;
	wtChar_.rotation_.y -= 0.01f;

	// 行列更新
	wtTitle_.matWorld_ = MakeAffineMatrix(wtTitle_.scale_, wtTitle_.rotation_, wtTitle_.translation_);
	wtTitle_.TransferMatrix();

	wtChar_.matWorld_ = MakeAffineMatrix(wtChar_.scale_, wtChar_.rotation_, wtChar_.translation_);
	wtChar_.TransferMatrix();

	// スペースキーでゲーム開始
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(new GameScene());
	}
}

void TitleScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 2D描画（背景などあればここで）
	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();

	// 3D描画
	Model::PreDraw(dxCommon->GetCommandList());

	if (modelTitle_)
		modelTitle_->Draw(wtTitle_, camera_);
	if (modelChar_)
		modelChar_->Draw(wtChar_, camera_);

	Model::PostDraw();

}

TitleScene::~TitleScene() {
	delete modelTitle_;
	delete modelChar_;
}