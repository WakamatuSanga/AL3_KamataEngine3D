#include "GameScene.h"
#include "MyMath.h"
using namespace KamataEngine;

void GameScene::Initialize()
{ 
	//textureHandle_ = TextureManager::Load("inseki.png"); 
	////スプライトインスタンスの生成
	//spreite_ = Sprite::Create(textureHandle_, {100, 50});
	////3Dモデルの生成
	//model_ = Model::Create();
	//ワールドトランスフォームの初期化
	//worldTransform_.Initialize();
	//カメラの初期化
	camera_.Initialize();
	modelBlock_ = Model::CreateFromOBJ("cube");
	/*const uint32_t kNumBlockHorizontal = 20;
	const float kBlockWidth = 2.0f;
	worldTransformBlocks_.resize(kNumBlockHorizontal);

	for (uint32_t i = 0; i < kNumBlockHorizontal; i++) {
		worldTransformBlocks_[i] = new WorldTransform();
		worldTransformBlocks_[i] -> Initialize();
		worldTransformBlocks_[i]->translation_.x = kBlockWidth * i;
		worldTransformBlocks_[i]->translation_.y = 0.0f;
	}*/

	const uint32_t kNumBlockVirticrl = 10;
	const uint32_t kNumBlockHorezontal = 20;

	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;


	worldTransformBlocks_.resize(kNumBlockVirticrl);
	for (uint32_t i = 0; i < kNumBlockVirticrl; ++i) {
		worldTransformBlocks_[i].resize(kNumBlockHorezontal);
	}

	for (uint32_t i = 0; i < kNumBlockVirticrl; i++) {
		for (uint32_t j = 1; j < kNumBlockHorezontal; j++) {
			if (j % 2 == 0)
				continue;
			if (i % 2 == 0)
				continue;
			
			worldTransformBlocks_[i][j] = new WorldTransform();
			worldTransformBlocks_[i][j]->Initialize();
			worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
			worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;

		}
	}
	//デバックカメラ生成
	debugCamera_ = new DebugCamera(static_cast <int>(kBlockHeight), static_cast <int>(kBlockWidth));
}

void GameScene::Update() 
{
	////スプライトの今の座標を取得
	//Vector2 position = spreite_->GetPosition();
	////座標を{2,1}移動
	//position.x += 2.0f;
	//position.y += 1.0f;
	////移動した座標をスプライトに反映
	//spreite_->SetPosition(position);
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) 
				continue;
			
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			worldTransformBlock->TransferMatrix();
		}
	}
	// デバックカメラの更新
	debugCamera_->Update();

	#ifdef  _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif //  _DEBUG

	//カメラの処理
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}
}

void GameScene::Draw() 
{ 
	//DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance(); 
	//スプライト描画後処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	/*spreite_->Draw();*/

	//スプライト描画後処理
	Sprite::PostDraw();

	//3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			modelBlock_->Draw(*worldTransformBlock, camera_);
			
			
		}
	}
	
	////3Dモデル描画
	//model_->Draw(worldTransform_, camera_, textureHandle_);

	//3Dモデル描画後処理
	Model::PostDraw();
	
}

GameScene::~GameScene() 
{
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	
	worldTransformBlocks_.clear();
	delete spreite_; 
	delete model_;
	delete debugCamera_;
}
