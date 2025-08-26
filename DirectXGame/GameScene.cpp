#include "GameScene.h"
#include "Math.h"
#include <random>

using namespace KamataEngine;

namespace {
constexpr float kDt = 1.0f / 60.0f; // 固定フレーム時間
}

GameScene::~GameScene() {

	// 先に敵を確実に破棄
	ClearEnemies();

	ClearItems();
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

	delete pauseDim_;
	pauseDim_ = nullptr;
	delete pauseBtnA_;
	pauseBtnA_ = nullptr;
	delete pauseBtnB_;
	pauseBtnB_ = nullptr;
}

void GameScene::SpawnHitEffect(const Vector3& pos) {
	auto* p = new DeathParticles();
	Vector4 col = {1.0f, 0.9f, 0.2f, 1.0f}; // 黄
	float speed = 0.08f;
	float dur = 0.20f;
	p->Initialize(deathParticle_model_, &camera_, pos, speed, dur, col);
	hitPfx_.push_back(p);
}

void GameScene::Initialize() {

	rng_.seed(std::random_device{}()); 
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
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(15, 16);
	player_->SetMapChipField(mapChipField_);
	player_->Initialize(player_model_, &camera_, playerPosition);

	// ★ 攻撃モデルを読み込み、プレイヤーへ渡す（OBJ名は手持ちに合わせて）
	attack_model_ = Model::CreateFromOBJ("slash", true); // 例：Resources/ slash / slash.obj
	player_->SetAttackModel(attack_model_);

	cleared_ = false; // ★ 追加

	// アイテムモデル
	item_model_ = Model::CreateFromOBJ("item"); // リソース名はお好みで

	// アイテム配置（互いに近すぎないように）
	SpawnItemsRandom(kItemCount_);

	// カメラコントローラ
	CController_ = new CameraController();
	CController_->Initialize(&camera_);
	CController_->SetTarget(player_);
	CController_->Reset();
	float yMin = 6.0f;
	float yMax = static_cast<float>(mapChipField_->GetNumBlockVirtical() - 1);

	// X は既存どおり左右の安全域、Y は全体に可動域を持たせる
	CameraController::Rect cameraArea = {
	    12.0f,          // left
	    100.0f - 12.0f, // right
	    yMin,           // bottom（小さい方）
	    yMax            // top（大きい方）
	};
	CController_->SetMovableArea(cameraArea);

	// 敵
	enemy_model_ = Model::CreateFromOBJ("enemy");
	// ★ 乱数の初期化
	rng_.seed(std::random_device{}());

	SpawnRandomEnemies(12);

	// デスパーティクル
	deathParticle_model_ = Model::CreateFromOBJ("deathParticle");

	// フェーズ＆フェード
	phase_ = Phase::kFadeIn;
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	
// ===== ポーズUI =====
	pauseDim_ = Sprite::Create(0, {0, 0});
	pauseDim_->SetSize({(float)WinApp::kWindowWidth, (float)WinApp::kWindowHeight});
	pauseDim_->SetColor({0, 0, 0, 0.6f});

	pauseBtnA_ = Sprite::Create(0, {(float)WinApp::kWindowWidth / 2 - 140.0f, (float)WinApp::kWindowHeight / 2 - 30.0f});
	pauseBtnA_->SetSize({280, 40});

	pauseBtnB_ = Sprite::Create(0, {(float)WinApp::kWindowWidth / 2 - 140.0f, (float)WinApp::kWindowHeight / 2 + 30.0f});
	pauseBtnB_->SetSize({280, 40});

	paused_ = false;
	pauseIndex_ = 0;
	requestReturnToTitle_ = false;

	// ===== HPバー作成（内蔵白テクスチャを使用）=====
	hpBg_ = Sprite::Create(0, {20, 20}); // 左上
	hpBg_->SetSize({220, 20});
	hpBg_->SetColor({0, 0, 0, 0.5f}); // 半透明の黒

	hpFg_ = Sprite::Create(0, {25, 25}); // 少し内側
	hpFg_->SetSize({210, 10});
	hpFg_->SetColor({0.85f, 0.2f, 0.2f, 1.0f}); // 赤

	// 乱数初期化（復活ディレイ / スポーン位置で使う） rng_.seed(std::random_device{}());

	// 初期スポーン（既存）
	enemy_model_ = Model::CreateFromOBJ("enemy");
	for (int i = 0; i < 12; ++i) {
		SpawnEnemyRandomVertical();
	}

	
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

	// フェーズ切り替え（死亡→フェードアウトなど）
	ChangePhase();

	switch (phase_) {

	case Phase::kFadeIn: {
		// フェードイン進行
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		// 背景・カメラ
		skydome_->Update();
		CController_->Update();

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

		// ブロックの行列更新
		for (auto& line : worldTransformBlocks_) {
			for (WorldTransform*& wt : line) {
				if (!wt)
					continue;
				WorldTransformUpdate(*wt);
			}
		}
		break;
	}

	case Phase::kPlay: {

		// --- ポーズトグル ---
		if (Input::GetInstance()->TriggerKey(DIK_ESCAPE) || Input::GetInstance()->TriggerKey(DIK_P)) {
			paused_ = !paused_;
		}

		// --- ポーズ中の入力＆スキップ ---
		if (paused_) {
			if (Input::GetInstance()->TriggerKey(DIK_UP))
				pauseIndex_ = 0;
			if (Input::GetInstance()->TriggerKey(DIK_DOWN))
				pauseIndex_ = 1;

			if (Input::GetInstance()->TriggerKey(DIK_RETURN) || Input::GetInstance()->TriggerKey(DIK_SPACE)) {
				if (pauseIndex_ == 0) {
					paused_ = false; // Resume
				} else {
					// Titleへ戻る要求 → フェードアウト移行
					requestReturnToTitle_ = true;
					fade_->Start(Fade::Status::FadeOut, 0.8f);
					phase_ = Phase::kFadeOut;
				}
			}

			// 行列だけ整えてこのフレームの通常更新を止める
			camera_.UpdateMatrix();
			for (auto& line : worldTransformBlocks_) {
				for (WorldTransform*& wt : line) {
					if (wt)
						WorldTransformUpdate(*wt);
				}
			}
			break; // ★この case を抜ける
		}

		// 背景・カメラコントローラ
		skydome_->Update();
		CController_->Update();

		// プレイヤー・敵の更新
		player_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		// 撃破済みを「復活予約 → 削除」
		for (auto it = enemies_.begin(); it != enemies_.end();) {
			Enemy* e = *it;
			if (e->IsDead()) {
				// 倒した高さ（ワールドY）で復活予約
				const float yWorld = e->GetWorldPosition().y;
				EnqueueRespawnAt(yWorld);
				delete e;
				it = enemies_.erase(it);
			} else {
				++it;
			}
		}

		// 予約消化（固定Δt。GameScene.cpp先頭に constexpr float kDt = 1.0f/60.0f; を定義）
		UpdateRespawns(kDt);

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

		// ブロックの行列更新
		for (auto& line : worldTransformBlocks_) {
			for (WorldTransform*& wt : line) {
				if (!wt)
					continue;
				WorldTransformUpdate(*wt);
			}
		}

		// 衝突判定
		CheckAllCollisions();
		// プレイヤー・敵の更新の後に
		UpdateItems();

		// 既存のプレイヤー/敵の当たり判定
		CheckAllCollisions();

		for (auto it = hitPfx_.begin(); it != hitPfx_.end();) {
			auto* p = *it;
			p->Update();
			if (p->IsFinished()) {
				delete p;
				it = hitPfx_.erase(it);
			} else {
				++it;
			}
		}

		// ★ 全取得チェック → クリアフェードへ
		if (!cleared_ && AreAllItemsCollected()) {
			cleared_ = true;
			fade_->Start(Fade::Status::FadeOut, 1.0f); // 画面を暗く
			phase_ = Phase::kClearFadeOut;
		}

		break;
	}
		
	case Phase::kDeath: {
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
		for (auto it = hitPfx_.begin(); it != hitPfx_.end();) {
			auto* p = *it;
			p->Update();
			if (p->IsFinished()) {
				delete p;
				it = hitPfx_.erase(it);
			} else {
				++it;
			}
		}
		// 死亡演出中も復活タイマーだけは進める（不要ならコメントアウト）
		UpdateRespawns(kDt);
		break;
	}
		

	case Phase::kFadeOut: {
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		skydome_->Update();
		CController_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		// ※ フェードアウト中は基本スポーンさせない想定なので UpdateRespawns は呼ばない
		break;
	}

	case Phase::kClearFadeOut: {
		// クリア演出用フェードアウト
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true; // シーン終了 → WinMain 側で ClearScene へ
		}
		// 背景等のアップデートは最低限
		skydome_->Update();
		CController_->Update();
		break;
	}

	default:
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

	for (auto* p : hitPfx_) {
		p->Draw();
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

	// 3D
	Model::PreDraw(dxCommon->GetCommandList());

	if (!player_->IsDead())
		player_->Draw();

	// ★ アイテムを描く
	for (Item* it : items_)
		it->Draw();

	skydome_->Draw();
	// ... ブロック・敵・パーティクル ...
	Model::PostDraw();

	if (paused_) {
		DirectXCommon* dx = DirectXCommon::GetInstance();
		Sprite::PreDraw(dx->GetCommandList());

		const Vector4 sel = {0.9f, 0.9f, 0.9f, 1.0f};
		const Vector4 unsel = {0.4f, 0.4f, 0.4f, 1.0f};

		pauseDim_->Draw();

		pauseBtnA_->SetColor(pauseIndex_ == 0 ? sel : unsel); // Resume
		pauseBtnA_->Draw();

		pauseBtnB_->SetColor(pauseIndex_ == 1 ? sel : unsel); // Title
		pauseBtnB_->Draw();

		Sprite::PostDraw();
	}

	// フェード（フェード中はUIが黒で隠れる：UIを前面に出したい場合はこの下で再度Spriteパスを回す）
	fade_->Draw();
}

void GameScene::ClearItems() {
	for (Item* it : items_)
		delete it;
	items_.clear();
}

void GameScene::UpdateItems() {
	// 取得判定：プレイヤーAABB とアイテムAABB
	const AABB playerBox = player_->GetAABB();
	for (Item* it : items_) {
		if (it->IsCollected())
			continue;
		it->Update();
		if (IsCollision(playerBox, it->GetAABB())) {
			it->Collect();
			// ここでSEやエフェクトを出したければ呼ぶ
			// Audio::GetInstance()->PlayWave(pickSeHandle_);
		}
	}
}

bool GameScene::AreAllItemsCollected() const {
	for (const Item* it : items_) {
		if (!it->IsCollected())
			return false;
	}
	return true;
}

// 互いに近すぎないようにランダム配置
void GameScene::SpawnItemsRandom(int count) {
	if (!mapChipField_)
		return;

	const uint32_t H = mapChipField_->GetNumBlockVirtical();
	const uint32_t W = mapChipField_->GetNumBlockHorizontal();

	// 候補セル収集：「空白 かつ ひとつ下がブロック」＝地面上
	std::vector<Vector3> candidates;
	candidates.reserve(H * W);
	for (uint32_t y = 0; y + 1 < H; ++y) {
		for (uint32_t x = 1; x + 1 < W; ++x) { // 端を避ける
			if (mapChipField_->GetMapChipTypeByIndex(x, y) != MapChipType::kBlank)
				continue;
			if (mapChipField_->GetMapChipTypeByIndex(x, y + 1) != MapChipType::kBlock)
				continue;
			candidates.push_back(mapChipField_->GetMapChipPositionByIndex(x, y));
		}
	}
	if (candidates.empty())
		return;

	// シャッフル
	std::shuffle(candidates.begin(), candidates.end(), rng_);

	// 貪欲に距離条件を満たすものを選ぶ
	std::vector<Vector3> chosen;
	for (const Vector3& c : candidates) {
		bool ok = true;
		for (const Vector3& d : chosen) {
			float dx = c.x - d.x;
			float dy = c.y - d.y;
			if (dx * dx + dy * dy < kItemMinDist_ * kItemMinDist_) {
				ok = false;
				break;
			}
		}
		if (ok) {
			chosen.push_back(c);
			if ((int)chosen.size() >= count)
				break;
		}
	}
	// 候補が足りなければ、距離条件を少し緩めたい場合はここで再挑戦ロジックを追加可

	for (const Vector3& p : chosen) {
		Item* it = new Item();
		// ★ ランダム化ここから
		std::uniform_real_distribution<float> spin(1.5f, 3.5f);        // 回転速度[rad/s]
		std::uniform_real_distribution<float> bobA(0.18f, 0.35f);      // ふわふわ振幅
		std::uniform_real_distribution<float> bobW(1.6f, 2.6f);        // ふわふわ角速度[rad/s]
		std::uniform_real_distribution<float> phase(0.0f, 6.2831853f); // 初期位相[0,2π]

		it->SetSpinSpeed(spin(rng_));
		it->SetBob(bobA(rng_), bobW(rng_));
		it->SetBobPhase(phase(rng_));
		// ★ ランダム化ここまで
		it->Initialize(item_model_, &camera_, p);
		items_.push_back(it);
	}
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
	// ▼ 追加：プレイヤー攻撃 vs 敵
	if (player_->IsAttackActive()) {
		AABB atk = player_->GetAttackAABB();
		for (Enemy* enemy : enemies_) {
			AABB eaabb = enemy->GetAABB();
			if (IsCollision(atk, eaabb)) {
				// 攻撃方向（ノックバック用）
				float dir = (player_->GetWorldTransform().rotation_.y < std::numbers::pi_v<float>) ? +1.0f : -1.0f;
				enemy->OnDamage(player_->GetAttackDamage(), Vector3(dir, 0, 0));
				// 小さめのヒットエフェクトを生成（黄色っぽく、短命）
				SpawnHitEffect(enemy->GetWorldPosition());

				// 小さめのカメラシェイク
				if (CController_)
					CController_->AddShake(0.12f, 0.08f);
			}
		}
	}

	//// 既存：プレイヤー本体 vs 敵の接触
	//{
	//	AABB aabb1 = player_->GetAABB();
	//	for (Enemy* enemy : enemies_) {
	//		AABB aabb2 = enemy->GetAABB();
	//		if (IsCollision(aabb1, aabb2)) {
	//			player_->OnCollision(enemy);
	//			enemy->OnCollision(player_);
	//		}
	//	}
	//}
}

void GameScene::EnqueueRespawnAt(float yWorld) {
	std::uniform_real_distribution<float> respawnSec(kRespawnDelayMin_, kRespawnDelayMax_);
	respawnQueue_.push_back({respawnSec(rng_), yWorld});
}

void GameScene::UpdateRespawns(float dt) {
	for (auto it = respawnQueue_.begin(); it != respawnQueue_.end();) {
		it->timer -= dt;
		if (it->timer <= 0.0f) {
			SpawnEnemyRandomVertical();
			it = respawnQueue_.erase(it);
		} else {
			++it;
		}
	}
}

// プレイヤーと同じ“高さ帯”で、空きマスをランダムに選んで出現
void GameScene::SpawnEnemyAtSameY(float yWorld) {
	// プレイヤー位置
	const Vector3 playerPos = player_->GetWorldPosition();

	// yWorld → タイルの yIndex（安全にクランプ）
	MapChipField::IndexSet idxY = mapChipField_->GetMapChipIndexSetByPosition(Vector3(0.0f, yWorld, 0.0f));
	const uint32_t yIndex = std::clamp(idxY.yIndex, 0u, mapChipField_->GetNumBlockVirtical() - 1);

	// 端の1列は避ける（衝突などの都合）
	const uint32_t xMin = 1u;
	const uint32_t xMax = mapChipField_->GetNumBlockHorizontal() - 2u;

	const float minDist = kRespawnMinDistance_; // 例: 8.0f
	const float minDist2 = minDist * minDist;

	std::vector<uint32_t> good; // 安全距離を満たす候補
	uint32_t farthestX = xMin;  // 最遠候補（保険）
	float farthestD2 = -1.0f;

	// 行全体を走査して、安全距離を満たす“空白タイル”だけ集める
	for (uint32_t x = xMin; x <= xMax; ++x) {
		if (mapChipField_->GetMapChipTypeByIndex(x, yIndex) != MapChipType::kBlank)
			continue;

		Vector3 pos = mapChipField_->GetMapChipPositionByIndex(x, yIndex);
		float dx = pos.x - playerPos.x;
		float dy = pos.y - playerPos.y; // 同じ高さなら 0
		float d2 = dx * dx + dy * dy;

		if (d2 >= minDist2) {
			good.push_back(x);
		}
		if (d2 > farthestD2) {
			farthestD2 = d2;
			farthestX = x;
		}
	}

	if (!good.empty()) {
		// 安全距離を満たす候補からランダム
		std::uniform_int_distribution<size_t> pick(0, good.size() - 1);
		SpawnEnemyAtIndex(good[pick(rng_)], yIndex);
		return;
	}

	if (farthestD2 >= 0.0f) {
		// 安全距離を満たす候補が無い → 行の中で“最も遠い”地点に出す（近場強制スポーン回避）
		SpawnEnemyAtIndex(farthestX, yIndex);
		return;
	}

	// 行がすべて埋まっている → 少し遅らせて再試行
	respawnQueue_.push_back({0.75f, yWorld});
}

void GameScene::SpawnEnemyRandomVertical() {
	// プレイヤー位置（安全距離チェックに使う）
	const Vector3 playerPos = player_->GetWorldPosition();

	const uint32_t H = mapChipField_->GetNumBlockVirtical();
	const uint32_t W = mapChipField_->GetNumBlockHorizontal();

	const uint32_t xMin = 1u;
	const uint32_t xMax = (W >= 2) ? (W - 2u) : 0u;

	const float minDist2 = kRespawnMinDistance_ * kRespawnMinDistance_;

	// 候補（安全距離を満たす）と最遠候補（保険）
	std::vector<std::pair<uint32_t, uint32_t>> good;
	std::pair<uint32_t, uint32_t> farthest = {xMin, 0};
	float farthestD2 = -1.0f;

	// 全行を走査して「空白 & ひとつ下がブロック」かつ安全距離の候補を集める
	for (uint32_t y = 0; y + 1 < H; ++y) {
		for (uint32_t x = xMin; x <= xMax; ++x) {
			if (mapChipField_->GetMapChipTypeByIndex(x, y) != MapChipType::kBlank)
				continue;
			if (mapChipField_->GetMapChipTypeByIndex(x, y + 1) != MapChipType::kBlock)
				continue;

			Vector3 pos = mapChipField_->GetMapChipPositionByIndex(x, y);
			float dx = pos.x - playerPos.x;
			float dy = pos.y - playerPos.y;
			float d2 = dx * dx + dy * dy;

			if (d2 >= minDist2) {
				good.emplace_back(x, y);
			}
			if (d2 > farthestD2) {
				farthestD2 = d2;
				farthest = {x, y};
			}
		}
	}

	if (!good.empty()) {
		// 候補からランダムに1つ
		std::uniform_int_distribution<size_t> pick(0, good.size() - 1);
		auto [gx, gy] = good[pick(rng_)];
		SpawnEnemyAtIndex(gx, gy);
		return;
	}
	if (farthestD2 >= 0.0f) {
		// 安全距離を満たす候補が無い → 最も遠い地点に出す（“近すぎ湧き”を回避）
		SpawnEnemyAtIndex(farthest.first, farthest.second);
		return;
	}

	// すべて埋まっている → 少し待って再試行
	respawnQueue_.push_back({0.75f, playerPos.y}); // yは使わないが既存構造に合わせる
}

void GameScene::SpawnEnemyAtIndex(uint32_t xIndex, uint32_t yIndex) {
	Vector3 pos = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
	Enemy* newEnemy = new Enemy();
	newEnemy->Initialize(enemy_model_, &camera_, pos);

	// ★追加：初期スポーンと同様にセット
	newEnemy->SetMapChipField(mapChipField_);
	newEnemy->SetTarget(player_);
	// 型ランダム
	std::uniform_real_distribution<float> coin(0.0f, 1.0f);
	float r = coin(rng_);
	if (r < 0.40f)
		newEnemy->SetType(Enemy::Type::Walker);
	else if (r < 0.65f)
		newEnemy->SetType(Enemy::Type::Jumper);
	else if (r < 0.90f)
		newEnemy->SetType(Enemy::Type::Chaser);
	else
		newEnemy->SetType(Enemy::Type::Flyer);

	enemies_.push_back(newEnemy);
}
