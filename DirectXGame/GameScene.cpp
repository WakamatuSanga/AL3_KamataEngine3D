#include "GameScene.h"

// 初期化
void GameScene::Initialize() {
	// カメラの初期化
	camera_.Initialize();
	camera_.translation_.z = -15.0f; // カメラを少し後ろに引く
	camera_.translation_.y = 5.0f;   // カメラを少し上に見下ろすように

	// プレイヤーモデルの読み込みと初期化
	playerModel_ = Model::CreateFromOBJ("player"); // player.objを読み込む
	player_ = new Player();
	player_->Initialize(playerModel_, &camera_);

	// 床モデルの読み込みと初期化
	floorModel_ = Model::CreateFromOBJ("block"); // block.objを読み込む
	floorWorldTransform_.Initialize();
	floorWorldTransform_.scale_ = {10.0f, 0.5f, 1.0f}; // 横長の床にする
	floorWorldTransform_.TransferMatrix();
}

// 更新
void GameScene::Update() {
	// プレイヤーの更新
	player_->Update();
}

// 描画
void GameScene::Draw() {
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// プレイヤーの描画
	player_->Draw();
	// 床の描画
	floorModel_->Draw(floorWorldTransform_, camera_);

	// 3Dモデル描画後処理
	Model::PostDraw();
}

// デストラクタ（終了処理）
GameScene::~GameScene() {
	delete player_;
	delete playerModel_;
	delete floorModel_;
}