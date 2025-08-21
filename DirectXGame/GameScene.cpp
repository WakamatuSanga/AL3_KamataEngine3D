#include "GameScene.h"
#include "Math.h"
#include <random>

using namespace KamataEngine;

GameScene::~GameScene() {

	// 先に敵を確実に破棄
	ClearEnemies();

	// ▼ HPバー破棄
	delete hpBg_;
	delete hpFg_;
	hpBg_ = nullptr;
	hpFg_ = nullptr;
	// ▲

	delete sprite_;
	delete model_;

	delete block_model_;
	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* wt : line) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;

	delete deathParticles_;
	delete deathParticle_model_;
}

void GameScene::Initialize() {

	// 再利用ケースでも二重スポーンを防ぐ
	ClearEnemies();

	// テクスチャ・スプライト
	textureHandle_ = TextureManager::Load("sample.png");
	sprite_ = Sprite::Create(textureHandle_, {100, 50});

	// 3Dモデル
	model_ = Model::Create();
	worldTransform_.Initialize();

	// カメラ
	camera_.Initialize();

	// ブロックモデル
	block_model_ = Model::CreateFromOBJ("block");

	// デバッグカメラ
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 天球
	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("SkyDome", true);
	skydome_->Initialize(modelSkydome_, &camera_);

	// マップ
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// プレイヤー
	player_ = new Player();
	player_model_ = Model::CreateFromOBJ("player");
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);
	player_->SetMapChipField(mapChipField_);
	player_->Initialize(player_model_, &camera_, playerPosition);

	// カメラコントローラ
	CController_ = new CameraController();
	CController_->Initialize(&camera_);
	CController_->SetTarget(player_);
	CController_->Reset();
	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	CController_->SetMovableArea(cameraArea);

	// 敵
	enemy_model_ = Model::CreateFromOBJ("enemy");
	SpawnRandomEnemies(12);

	// デスパーティクル
	deathParticle_model_ = Model::CreateFromOBJ("deathParticle");

	// フェーズ＆フェード
	phase_ = Phase::kFadeIn;
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// ===== HPバー作成（内蔵白テクスチャを使用）=====
	hpBg_ = Sprite::Create(0, {20, 20}); // 左上
	hpBg_->SetSize({220, 20});
	hpBg_->SetColor({0, 0, 0, 0.5f}); // 半透明の黒

	hpFg_ = Sprite::Create(0, {25, 25}); // 少し内側
	hpFg_->SetSize({210, 10});
	hpFg_->SetColor({0.85f, 0.2f, 0.2f, 1.0f}); // 赤
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3& pos = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticle_model_, &camera_, pos);
		}
		break;
	case Phase::kDeath:
		break;
	default:
		break;
	}
}

void GameScene::GenerateBlocks() {
	uint32_t H = mapChipField_->GetNumBlockVirtical();
	uint32_t W = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(H);
	for (uint32_t i = 0; i < H; ++i) {
		worldTransformBlocks_[i].resize(W);
	}

	for (uint32_t i = 0; i < H; ++i) {
		for (uint32_t j = 0; j < W; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				auto* wt = new WorldTransform();
				wt->Initialize();
				worldTransformBlocks_[i][j] = wt;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::ClearEnemies() {
	for (Enemy* e : enemies_) {
		delete e;
	}
	enemies_.clear();
}

void GameScene::SpawnRandomEnemies(int total) {
	if (!mapChipField_ || !player_)
		return;

	const uint32_t H = mapChipField_->GetNumBlockVirtical();
	const uint32_t W = mapChipField_->GetNumBlockHorizontal();

	MapChipField::IndexSet pIdx = mapChipField_->GetMapChipIndexSetByPosition(player_->GetWorldPosition());
	uint32_t playerBandY = pIdx.yIndex;

	// プレイヤーの近傍行で“空白＆1つ下がブロック”の段を優先（±2段チェック）
	{
		bool found = false;
		for (int dy = -2; dy <= 2 && !found; ++dy) {
			int y = static_cast<int>(pIdx.yIndex) + dy;
			if (y < 0 || y + 1 >= static_cast<int>(H))
				continue;
			uint32_t x = pIdx.xIndex;
			if (mapChipField_->GetMapChipTypeByIndex(x, y) == MapChipType::kBlank && mapChipField_->GetMapChipTypeByIndex(x, y + 1) == MapChipType::kBlock) {
				playerBandY = static_cast<uint32_t>(y);
				found = true;
			}
		}
	}

	auto collectSpawnsAtY = [&](uint32_t y, std::vector<std::pair<uint32_t, uint32_t>>& out) {
		if (y >= H)
			return;
		for (uint32_t x = 0; x < W; ++x) {
			if (mapChipField_->GetMapChipTypeByIndex(x, y) != MapChipType::kBlank)
				continue;
			if (y + 1 < H && mapChipField_->GetMapChipTypeByIndex(x, y + 1) == MapChipType::kBlock) {
				out.emplace_back(x, y);
			}
		}
	};

	std::vector<std::pair<uint32_t, uint32_t>> spawns;
	collectSpawnsAtY(playerBandY, spawns);

	if (spawns.empty()) {
		if (playerBandY > 0)
			collectSpawnsAtY(playerBandY - 1, spawns);
		if (spawns.empty() && playerBandY + 1 < H)
			collectSpawnsAtY(playerBandY + 1, spawns);
		if (spawns.empty()) {
			for (uint32_t y = 0; y < H; ++y)
				collectSpawnsAtY(y, spawns);
		}
	}
	if (spawns.empty())
		return;

	std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<size_t> pick(0, spawns.size() - 1);
	std::uniform_real_distribution<float> coin(0.0f, 1.0f);

	for (int i = 0; i < total; ++i) {
		auto [sx, sy] = spawns[pick(rng)];
		Vector3 pos = mapChipField_->GetMapChipPositionByIndex(sx, sy);

		Enemy* e = new Enemy();
		e->Initialize(enemy_model_, &camera_, pos);
		e->SetMapChipField(mapChipField_);
		e->SetTarget(player_);

		float r = coin(rng);
		if (r < 0.40f)
			e->SetType(Enemy::Type::Walker);
		else if (r < 0.65f)
			e->SetType(Enemy::Type::Jumper);
		else if (r < 0.90f)
			e->SetType(Enemy::Type::Chaser);
		else
			e->SetType(Enemy::Type::Flyer);

		enemies_.push_back(e);
	}
}

void GameScene::Update() {

	ChangePhase();

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		skydome_->Update();
		CController_->Update();

		player_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			camera_.TransferMatrix();
		} else {
			camera_.UpdateMatrix();
		}

		for (auto& line : worldTransformBlocks_) {
			for (WorldTransform*& wt : line) {
				if (!wt)
					continue;
				WorldTransformUpdate(*wt);
			}
		}
		break;

	case Phase::kPlay:
		skydome_->Update();
		CController_->Update();

		player_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			camera_.TransferMatrix();
		} else {
			camera_.UpdateMatrix();
		}

		for (auto& line : worldTransformBlocks_) {
			for (WorldTransform*& wt : line) {
				if (!wt)
					continue;
				WorldTransformUpdate(*wt);
			}
		}

		CheckAllCollisions();
		break;

	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
		}

		skydome_->Update();
		CController_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		if (deathParticles_) {
			deathParticles_->Update();
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		skydome_->Update();
		CController_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		break;
	}
}

void GameScene::Draw() {

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3D
	Model::PreDraw(dxCommon->GetCommandList());

	if (!player_->IsDead())
		player_->Draw();

	skydome_->Draw();

	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform*& wt : line) {
			if (!wt)
				continue;
			block_model_->Draw(*wt, camera_);
		}
	}

	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	if (deathParticles_) {
		deathParticles_->Draw();
	}

	Model::PostDraw();

	// 2D（HPバー）
	Sprite::PreDraw(dxCommon->GetCommandList());

	// ★ HPバー更新（Player に HP 実装がある場合は GetHPRatio() を使う）
	float ratio = 1.0f;
	// ratio = player_->GetHPRatio();
	if (player_->IsDead())
		ratio = 0.0f;

	hpFg_->SetSize({210.0f * ratio, 10.0f});

	// 背景 → 本体 の順で描画
	hpBg_->Draw();
	hpFg_->Draw();

	Sprite::PostDraw();

	// フェード（フェード中はUIが黒で隠れる：UIを前面に出したい場合はこの下で再度Spriteパスを回す）
	fade_->Draw();
}

void GameScene::CheckAllCollisions() {

	AABB aabb1, aabb2;
	aabb1 = player_->GetAABB();

	for (Enemy* enemy : enemies_) {
		aabb2 = enemy->GetAABB();
		if (IsCollision(aabb1, aabb2)) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
}
