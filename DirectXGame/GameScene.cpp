#include "GameScene.h"
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize() {
	//カメラ
	camera_.Initialize();
	camera_.UpdateMatrix();
	
	// 描画クラスの生成
	PrimitiveDrawer::GetInstance()->Initialize();

	// レールカメラ
	railCamera_.Initialize(/*pos*/ {0, 2.0f, -10.0f}, /*rot*/ {0, 0, 0}, /*fovY*/ 0.45f, /*near*/ 0.1f, /*far*/ 5000.0f);
	
	// 変数の初期化
	phase_ = Phase::kWait;
	splineT_ = 0.0f;

// --- スプライン制御点（通過点） ---
	// 距離3000の超ロングコース
	splineControlPoints_ = {
	    // 【Start Buffer】
	    {0.0f,    0.0f,   -50.0f },

	    // 1. スタート
	    {0.0f,    0.0f,   0.0f   },

	    // 2. 助走 (Z=200まで加速)
	    {0.0f,    0.0f,   200.0f },

	    // 3. 第1上昇 (Z=600, Y=150)
	    {0.0f,    150.0f, 600.0f },

	    // 4. 右旋回しながら急降下 (Z=1000, Y=30, X=100)
	    {100.0f,  30.0f,  1000.0f},

	    // 5. 左へ切り返して急上昇 (Z=1500, Y=300 !!) 一気に空へ
	    {-100.0f, 300.0f, 1500.0f},

	    // 6. 頂上でキープ (Z=1800, Y=300)
	    {0.0f,    300.0f, 1800.0f},

	    // 7. 奈落へダイブ (Z=2200, Y=50)
	    {0.0f,    50.0f,  2200.0f},

	    // 8. 最後のひと山 (Z=2600, Y=150)
	    {50.0f,   150.0f, 2600.0f},

	    // 9. 着地してゴール (Z=3000)
	    {0.0f,    0.0f,   3000.0f},

	    // 10. ゴールライン
	    {0.0f,    0.0f,   3100.0f},

	    // 【End Buffer】
	    {0.0f,    0.0f,   3200.0f},
	};
	// 最初のサンプルを作っておく（無くても Update で毎フレ作るので OK）
	splinePoints_.clear();
	// 線分で描画するための頂点リスト計算
	splinePoints_.clear();

	// 分割数 (100個の線分)
	const size_t segmentCount = 400;

	// MyMathにある関数を使って、0.0～1.0の間を細かくサンプリングする
	for (size_t i = 0; i <= segmentCount; i++) {
		float t = (float)i / segmentCount;
		Vector3 pos = CatmullRomSpline(splineControlPoints_, t);

		splinePoints_.push_back(pos);
	}
	
	// 地面
	groundModel_ = Model::CreateFromOBJ("sea", true);
	ground_.Initialize(groundModel_);
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
	// ===============================================
	// ▼ フェーズ管理
	// ===============================================
	switch (phase_) {
	case Phase::kWait:
		timer_ += 1.0f / 60.0f; // 1フレームごとに時間を進める

		// 2秒経ったら移動フェーズへ
		if (timer_ >= 2.0f) {
			phase_ = Phase::kMove;
			splineT_ = 0.0f; // 移動開始位置をリセット
		}
		break;

	case Phase::kIntro:
		// 演出用フェーズ（今回はスキップしてPhase::kMoveへ流す例）
		phase_ = Phase::kMove;
		break;

	case Phase::kMove: // ここがレール移動フェーズ
	{
		// 1. 進行度 t を進める
		splineT_ += moveSpeed_;

		// 2. 終点についたらフェーズ終了
		if (splineT_ >= 1.0f) {
			splineT_ = 1.0f;
			phase_ = Phase::kEnd;
		}

		// 3. 座標の計算
		Vector3 lookTarget = CatmullRomSpline(splineControlPoints_, splineT_);

		// カメラ情報の取得
		WorldTransform& worldTransform = railCamera_.GetWorldTransform();

		// ★重要：ここで worldTransform.translation_ = ... を書かない！
		// これにより、カメラは初期位置（または待機位置）から動きません。

		// ★変更点：回転の計算
		// 「自分の現在地」から「レール上の点」へのベクトルを計算
		Vector3 diff = {lookTarget.x - worldTransform.translation_.x, lookTarget.y - worldTransform.translation_.y, lookTarget.z - worldTransform.translation_.z};

		// 水平方向の距離（上下角度の計算用）
		float horizontalDist = std::sqrt(diff.x * diff.x + diff.z * diff.z);

		if (horizontalDist != 0.0f) {
			// (1) Y軸回転（左右）：ターゲットの方角を向く
			worldTransform.rotation_.y = std::atan2(diff.x, diff.z);

			// (2) X軸回転（上下）：見上げる/見下ろす
			// マイナスをつけることで、高い場所を見るときにカメラが上を向くようになります
			worldTransform.rotation_.x = std::atan2(-diff.y, horizontalDist);
		}
		// レールカメラの更新処理（行列計算）を呼ぶ
		railCamera_.Update();
	} break;

	case Phase::kEnd:
		// 終了時の処理（止まるなど）
		break;
	}

	// 天球
	skydome_.Update();
	// プレイヤー
	player_.Update();
	// 地面更新
	ground_.Update();
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
