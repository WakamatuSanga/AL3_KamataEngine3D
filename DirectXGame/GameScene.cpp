#include "GameScene.h"

void GameScene::Initialize() {
	// カメラの初期化
	camera_.Initialize();
	camera_.translation_ = {15.0f, 10.0f, -30.0f}; // ステージ全体が見える位置に調整

	// プレイヤーモデルの読み込みと初期化
	playerModel_ = Model::CreateFromOBJ("player");
	player_ = new Player();
	player_->Initialize(playerModel_, &camera_);
	player_->worldTransform_.translation_ = {6.0f, 15.0f, 0.0f}; // 開始位置を調整

	// ブロックモデルの読み込み
	blockModel_ = Model::CreateFromOBJ("block");

	// マップチップフィールドの生成とCSV読み込み
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	// 読み込んだマップデータをコンソールに出力して確認する
	for (uint32_t y = 0; y < mapChipField_->GetNumBlockVertical(); ++y) {
		std::string line = "";
		for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
			if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
				line += "1,"; // ブロックなら1
			} else {
				line += "0,"; // 空白なら0
			}
		}
		line += "\n";
		OutputDebugStringA(line.c_str()); // 出力ウィンドウに表示
	}
	// CSVに基づいてブロックのWorldTransformを生成
	for (uint32_t y = 0; y < mapChipField_->GetNumBlockVertical(); ++y) {
		for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
			if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlock) {
				// WorldTransformを生成してvectorに追加
				KamataEngine::WorldTransform* wt = new KamataEngine::WorldTransform(); // newで生成
				wt->Initialize();
				wt->translation_ = mapChipField_->GetMapChipPositionByIndex(x, y);
				wt->scale_ = {1.0f, 1.0f, 1.0f};
				wt->TransferMatrix();
				blockWorldTransforms_.push_back(wt); // ポインタをvectorに追加
				
			}
		}
	}
}

void GameScene::Update() {
	camera_.UpdateMatrix();

	// プレイヤーの更新
	player_->Update();
}

void GameScene::Draw() {
	KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	// プレイヤーの描画
	player_->Draw();

	// 全てのブロックを描画
	for (auto& wt : blockWorldTransforms_) {
		blockModel_->Draw(*wt, camera_);
	}

	Model::PostDraw();
}

GameScene::~GameScene() {
	delete player_;
	delete playerModel_;
	delete blockModel_;
	delete mapChipField_;
	for (auto* wt : blockWorldTransforms_) {
		delete wt;
	}
}