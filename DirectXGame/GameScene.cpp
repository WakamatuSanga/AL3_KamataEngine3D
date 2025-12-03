#include "GameScene.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	//カメラ
	camera_.Initialize();
	camera_.UpdateMatrix();
	
	// 描画クラスの生成
	PrimitiveDrawer::GetInstance()->Initialize();

	// レールカメラ
	railCamera_.Initialize(/*pos*/ {0, 2.0f, -10.0f}, /*rot*/ {0, 0, 0}, /*fovY*/ 0.45f, /*near*/ 0.1f, /*far*/ 800.0f);
	
	// スプライン制御点（通過点）
	splineControlPoints_ = {
	    {0.0f,  0.0f,  40.0f},
        {10.0f, 10.0f, 40.0f},
        {10.0f, 15.0f, 40.0f},
        {20.0f, 15.0f, 40.0f},
        {20.0f, 0.0f,  40.0f},
        {30.0f, 0.0f,  40.0f},
	};

	// 最初のサンプルを作っておく（無くても Update で毎フレ作るので OK）
	splinePoints_.clear();
	// 線分で描画するための頂点リスト計算
	splinePoints_.clear();

	// 分割数 (100個の線分)
	const size_t segmentCount = 100;

	// MyMathにある関数を使って、0.0～1.0の間を細かくサンプリングする
	for (size_t i = 0; i <= segmentCount; i++) {
		float t = (float)i / segmentCount;
		Vector3 pos = CatmullRomSpline(splineControlPoints_, t);

		splinePoints_.push_back(pos);
	}
	
	// 地面
	groundModel_ = KamataEngine::Model::CreateFromOBJ("ground");
	ground_.InitializeOBJ(
	    groundModel_,
	    /*stepZ*/ 20.0f,
	    /*countZ*/ 16,
	    /*y*/ -15.0f,
	    /*speed*/ 0.6f,
	    /*uniformScale*/ 1.0f,
	    /*columns*/ 3,
	    /*colSpacingX*/ 10.0f // 列間隔（見た目に合わせて調整）
	);

	// 初期位置をカメラ手前から並べる（任意）
	ground_.StartAtCameraFront(/*cameraZ*/ 0.0f, /*tilesInFront*/ 4, /*margin*/ 2.0f);

	// カメラよりもっと後ろまで行ってから再配置（任意）
	ground_.UseBehindCameraRecycle(80.0f);
	ground_.SetCameraZ(0.0f); // Update前に毎フレーム

	// プレイヤーモデル（OBJ名はプロジェクトに合わせて）
	playerModel_ = Model::CreateFromOBJ("player"); // 無ければ "cube" など
	player_.Initialize(playerModel_);

	// プレイヤをレールカメラの子にして、カメラより前にオフセット
	player_.SetParent(&railCamera_.GetWorldTransform());
	// カメラの前方にローカル配置（+Zが前想定）
	player_.SetLocalPosition({0.0f, 0.0f, +15.0f});

	// 敵モデル（とりあえず cube を流用でもOK）
	enemyModel_ = Model::CreateFromOBJ("enemy"); // 無ければ "cube"
	enemy_ = new Enemy();
	enemy_->Initialize(enemyModel_);
	// 自機狙い敵（モデルは enemyModel_ を使い回しても、別OBJでもOK）
	enemyAimerModel_ = Model::CreateFromOBJ("enemy"); // 無ければ "enemy" など
	enemyAimer_ = new EnemyAimer();
	enemyAimer_->Initialize(enemyAimerModel_, &player_);
	// ホーミング敵（他と同じスタイル）
	enemyHomingModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyHomingModel_)
		enemyHomingModel_ = Model::Create();
	enemyHoming_ = new EnemyHoming();
	enemyHoming_->Initialize(enemyHomingModel_, &player_);

	// 天球モデルの生成と初期化
	modelSkydome_ = Model::CreateFromOBJ("skydome", true); // skydome/skydome.obj を読む
	if (!modelSkydome_)
		modelSkydome_ = Model::Create(); // フォールバック
	skydome_.Initialize(modelSkydome_);
}

void GameScene::Update() {
	
	//レールカメラ
	railCamera_.Update();
	// 天球
	skydome_.Update();
	// プレイヤー
	player_.Update();
	ground_.SetPlayerX(player_.GetPosition().x);
	// 地面更新
	ground_.Update();
	ground_.SetCameraZ(/*cameraのワールドZ*/ 0.0f);
	// エネミー群
	if (enemy_) {
		enemy_->Update();
	}
	if (enemyAimer_) {
		enemyAimer_->Update();
	}
	if (enemyHoming_) {
		enemyHoming_->Update();
	}

	// 毎フレーム最後に当たり判定
	CheckAllCollisions();
}

static inline float DistSq(const Vector3& a, const Vector3& b) {
	float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

void GameScene::CheckAllCollisions() {
	using KamataEngine::Vector3;

	auto distSq = [](const Vector3& a, const Vector3& b) {
		float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
		return dx * dx + dy * dy + dz * dz;
	};

	const Vector3 playerPos = player_.GetPosition();
	const float rPlayer = player_.GetCollisionRadius();
	const auto& pbs = player_.GetBullets();

	// ── 汎用：プレイヤー vs 任意の敵弾配列（半径は弾ごとに取得） ──
	auto collidePlayerVsBullets = [&](const auto& bullets) {
		for (auto* b : bullets) {
			if (!b || b->IsDead())
				continue;
			float rr = rPlayer + b->GetCollisionRadius();
			if (distSq(playerPos, b->GetPosition()) <= rr * rr) {
				player_.OnCollision();
				b->OnCollision(); // 弾は消す
			}
		}
	};

	// ── 汎用：自弾 vs 敵本体（自弾だけ消す） ──
	auto collidePlayerBulletsVsEnemyBody = [&](const Vector3& enemyPos, float rEnemy) {
		for (auto* pb : pbs) {
			if (!pb || pb->IsDead())
				continue;
			float rr = rEnemy + pb->GetCollisionRadius();
			if (distSq(enemyPos, pb->GetPosition()) <= rr * rr) {
				pb->OnCollision(); // 自弾は消す
				                   // 敵本体：必要なら OnCollision() を呼ぶ
			}
		}
	};

	// ── 汎用：自弾 vs 任意の敵弾配列（相殺） ──
	auto collidePlayerBulletsVsEnemyBullets = [&](const auto& bullets) {
		for (auto* pb : pbs) {
			if (!pb || pb->IsDead())
				continue;
			const Vector3& pa = pb->GetPosition();
			const float rPB = pb->GetCollisionRadius();
			for (auto* eb : bullets) {
				if (!eb || eb->IsDead())
					continue;
				float rr = rPB + eb->GetCollisionRadius();
				if (distSq(pa, eb->GetPosition()) <= rr * rr) {
					pb->OnCollision();
					eb->OnCollision();
				}
			}
		}
	};

	// ── 1) プレイヤー vs 敵弾（通常/自機狙い/ホーミング） ──
	if (enemy_)
		collidePlayerVsBullets(enemy_->GetBullets());
	if (enemyAimer_)
		collidePlayerVsBullets(enemyAimer_->GetBullets());
	if (enemyHoming_)
		collidePlayerVsBullets(enemyHoming_->GetBullets()); // EnemyHomingBullet*

	// ── 2) 自弾 vs 敵本体 ──
	if (enemy_) {
		collidePlayerBulletsVsEnemyBody(enemy_->GetPosition(), enemy_->GetCollisionRadius());
	}
	if (enemyAimer_) {
		collidePlayerBulletsVsEnemyBody(enemyAimer_->GetPosition(), enemyAimer_->GetCollisionRadius());
	}
	if (enemyHoming_) {
		collidePlayerBulletsVsEnemyBody(enemyHoming_->GetPosition(), enemyHoming_->GetCollisionRadius());
	}

	// ── 3) 自弾 vs 敵弾（相殺） ──
	if (enemy_)
		collidePlayerBulletsVsEnemyBullets(enemy_->GetBullets());
	if (enemyAimer_)
		collidePlayerBulletsVsEnemyBullets(enemyAimer_->GetBullets());
	if (enemyHoming_)
		collidePlayerBulletsVsEnemyBullets(enemyHoming_->GetBullets()); // EnemyHomingBullet*
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 2D
	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();

	// 3D
	Model::PreDraw(dxCommon->GetCommandList());
	// レールカメラの Camera を使う
	Camera& cam = railCamera_.GetCamera();
	

	// プレイヤー
	player_.Draw(cam);

	// エネミー群
	if (enemy_) {
		enemy_->Draw(cam);
	}
	if (enemyAimer_) {
		enemyAimer_->Draw(cam);
	}
	if (enemyHoming_) {
		enemyHoming_->Draw(cam);
	}
	// 天球
	skydome_.Draw(cam);
	// 地面
	ground_.Draw(cam);
	Model::PostDraw();

	if (splinePoints_.size() >= 2) {
		auto* drawer = PrimitiveDrawer::GetInstance();
		drawer->SetCamera(&railCamera_.GetCamera());

		Vector4 color{1.0f, 0.0f, 0.0f, 1.0f};
		for (size_t i = 1; i < splinePoints_.size(); ++i) {
			drawer->DrawLine3d(splinePoints_[i - 1], splinePoints_[i], color);
		}
	}
}

GameScene::~GameScene() {

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete groundModel_;
	groundModel_ = nullptr;

	delete enemy_;
	enemy_ = nullptr;

	delete enemyModel_;
	enemyModel_ = nullptr;

	delete enemyAimer_;
	enemyAimer_ = nullptr;

	delete enemyHoming_;
	enemyHoming_ = nullptr;

	delete enemyAimerModel_;
	enemyAimerModel_ = nullptr;

	delete enemyHomingModel_;
	enemyHomingModel_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;
}
