#include "GameOverScene.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "MyMath.h"

using namespace KamataEngine;

void GameOverScene::Initialize() {
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -15.0f};
	camera_.UpdateMatrix();

	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	if (!modelSkydome_) {
		modelSkydome_ = Model::Create();
	}
	skydome_.Initialize(modelSkydome_);

	modelText_ = Model::CreateFromOBJ("gameover_text");
	if (!modelText_)
		modelText_ = Model::Create();

	wtText_.Initialize();
	wtText_.translation_ = {0.0f, 2.0f, 0.0f};
	wtText_.scale_ = {1.5f, 1.5f, 1.5f};
	wtText_.rotation_ = {0.0f, 3.14f, 0.0f};
	modelPlayer_ = Model::CreateFromOBJ("player");
	if (!modelPlayer_)
		modelPlayer_ = Model::Create();

	wtPlayer_.Initialize();
	wtPlayer_.translation_ = {0.0f, -2.0f, 0.0f};
	// 倒れているように回転
	wtPlayer_.rotation_.x = 1.57f; // 90度
	wtPlayer_.rotation_.z = 0.5f;

	// BGM再生
	bgmHandle_ = Audio::GetInstance()->LoadWave("./Resources/BGM/bgm_gameover.wav");
	bgmVoiceHandle_ = Audio::GetInstance()->PlayWave(bgmHandle_, true);
	Audio::GetInstance()->SetVolume(bgmVoiceHandle_, 0.5f);
}

void GameOverScene::Update() {
	skydome_.Update();
	// テキストを揺らす（絶望感）
	float shake = (float)(rand() % 10 - 5) * 0.02f;
	wtText_.translation_.x = shake;

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

void GameOverScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();

	Model::PreDraw(dxCommon->GetCommandList());
	skydome_.Draw(camera_);
	if (modelText_)
		modelText_->Draw(wtText_, camera_);
	if (modelPlayer_)
		modelPlayer_->Draw(wtPlayer_, camera_);
	Model::PostDraw();
}

GameOverScene::~GameOverScene() {
	delete modelText_;
	delete modelPlayer_;
	delete modelSkydome_;

	if (Audio::GetInstance()->IsPlaying(bgmVoiceHandle_)) {
		Audio::GetInstance()->StopWave(bgmVoiceHandle_);
	}
}