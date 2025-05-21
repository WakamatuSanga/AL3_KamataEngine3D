#include "GameScene.h"
using namespace KamataEngine;

void GameScene::Initialize()
{ 
	textureHandle_ = TextureManager::Load("inseki.png"); 
	//スプライトインスタンスの生成
	spreite_ = Sprite::Create(textureHandle_, {100, 50});
	//3Dモデルの生成
	model_ = Model::Create();
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//カメラの初期化
	camera_.Initialize();
}

void GameScene::Update() 
{
	//スプライトの今の座標を取得
	Vector2 position = spreite_->GetPosition();
	//座標を{2,1}移動
	position.x += 2.0f;
	position.y += 1.0f;
	//移動した座標をスプライトに反映
	spreite_->SetPosition(position);
}

void GameScene::Draw() 
{ 
	//DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance(); 
	//スプライト描画後処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	spreite_->Draw();

	//スプライト描画後処理
	Sprite::PostDraw();

	//3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	//3Dモデル描画
	model_->Draw(worldTransform_, camera_, textureHandle_);

	//3Dモデル描画後処理
	Model::PostDraw();
}

GameScene::~GameScene() 
{
	delete spreite_; 
	delete model_;
}
