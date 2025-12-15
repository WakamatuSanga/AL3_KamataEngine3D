#include "GameScene.h"
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize() {
	// カメラ
	camera_.Initialize();
	camera_.UpdateMatrix();

	// 描画クラスの生成
	PrimitiveDrawer::GetInstance()->Initialize();

	// レールカメラ
	railCamera_.Initialize(/*pos*/ {0, 2.0f, -10.0f}, /*rot*/ {0, 0, 0}, /*fovY*/ 0.45f, /*near*/ 0.1f, /*far*/ 5000.0f);

	playerWorldTransform_.Initialize();

	// 変数の初期化
	phase_ = Phase::kWait;
	splineT_ = 0.0f;

	// --- スプライン制御点（通過点） ---
	// 高低差に加え、左右の蛇行（X軸）を激しくしたコース
	splineControlPoints_ = {
	    // 【Start Buffer】
	    {0.0f,    0.0f,   -50.0f },

	    // 1. スタート
	    {0.0f,    0.0f,   0.0f   },

	    // 2. 助走 (Z=200まで直進加速)
	    {0.0f,    0.0f,   200.0f },

	    // 3. 第1上昇 & 大きく右へ (X: 0 → 200, Y: 0 → 150)
	    {200.0f,  150.0f, 600.0f },

	    // 4. 左へ切り返しながら急降下 (X: 200 → -300 !!) 大回転
	    {-300.0f, 40.0f,  1000.0f},

	    // 5. 右へ切り返して急上昇 (X: -300 → 300 !!) 一気に空へ
	    {300.0f,  350.0f, 1500.0f},

	    // 6. 頂上で左へ流す (X: 300 → -150)
	    {-150.0f, 300.0f, 1800.0f},

	    // 7. 右旋回しながら奈落へダイブ (X: -150 → 200, Y: 300 → 40)
	    {200.0f,  40.0f,  2200.0f},

	    // 8. 最後のひと山 & 左へ (X: 200 → -200)
	    {-200.0f, 150.0f, 2600.0f},

	    // 9. 中央に戻りつつ着地 (Z=3000)
	    {0.0f,    0.0f,   3000.0f},

	    // 10. ゴールライン (水平維持)
	    {0.0f,    0.0f,   3100.0f},

	    // 【End Buffer】
	    {0.0f,    0.0f,   3200.0f},
	};
	// 最初のサンプルを作っておく（無くても Update で毎フレ作るので OK）
	splinePoints_.clear();
	// 線分で描画するための頂点リスト計算
	splinePoints_.clear();

	// 分割数 (100個の線分)
	const size_t segmentCount = 500;

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

	//// プレイヤをレールカメラの子にして、カメラより前にオフセット
	//player_.SetParent(&railCamera_.GetWorldTransform());
	//// カメラの前方にローカル配置（+Zが前想定）
	//player_.SetLocalPosition({0.0f, 0.0f, +15.0f});

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
	// 入力を取得
	auto* input = Input::GetInstance();

	// ==================================================
	// 1. デバッグモード切替 (0キー)
	// ==================================================
	if (input->TriggerKey(DIK_0)) {
		isDebugCamera_ = !isDebugCamera_; // ON/OFF切り替え

		// 切り替わった瞬間の初期化
		if (isDebugCamera_) {
			// 現在のカメラ角度を引き継ぐ（切り替え時にガクッとならないように）
			debugCameraRot_ = railCamera_.GetWorldTransform().rotation_;
			// 切り替え直後のマウス位置を記憶（視点飛び防止）
			preMousePos_ = input->GetMousePosition();
		}
	}

	// カメラの情報を取得（書き換え用）
	WorldTransform& worldTransform = railCamera_.GetWorldTransform();

	// ==================================================
	// 2. モードごとの処理分岐
	// ==================================================
	if (isDebugCamera_) {
		// ▼▼▼ デバッグカメラモード（自由に動ける） ▼▼▼

		// --- (A) マウスで視点変更 (左クリック中のみ) ---
		Vector2 mousePos = input->GetMousePosition();

		if (input->IsPressMouse(0)) { // 左クリックしている間
			// マウスの移動量を計算
			float moveX = mousePos.x - preMousePos_.x;
			float moveY = mousePos.y - preMousePos_.y;
			const float sensitivity = 0.01f; // 感度

			// 角度を加算（Y軸は横回転、X軸は縦回転）
			debugCameraRot_.y += moveX * sensitivity;
			debugCameraRot_.x += moveY * sensitivity;
		}

		// ★常に最新のマウス位置を保存（クリック開始時のズレ防止）
		preMousePos_ = mousePos;

		// 計算した角度をカメラに適用
		worldTransform.rotation_ = debugCameraRot_;

		// --- (B) キーボードで移動 (WASD + Space/Shift) ---
		const float moveSpeed = 1.0f; // 移動スピード
		Vector3 velocity = {0, 0, 0};

		// カメラが向いている方向（Y軸回転）に合わせて移動ベクトルを作る
		// (これをしないと、横を向いてもWキーで北に進んでしまう)
		Matrix4x4 matRotY = MakeRotateYMatrix(worldTransform.rotation_.y);

		if (input->PushKey(DIK_UP))
			velocity.z += moveSpeed; // 前
		if (input->PushKey(DIK_DOWN))
			velocity.z -= moveSpeed; // 後
		if (input->PushKey(DIK_LEFT))
			velocity.x -= moveSpeed; // 左
		if (input->PushKey(DIK_RIGHT))
			velocity.x += moveSpeed; // 右

		// 移動方向をカメラの向きに合わせて回転
		velocity = TransformNormal(velocity, matRotY);

		// 上下移動
		if (input->PushKey(DIK_SPACE))
			velocity.y += moveSpeed; // 上昇
		if (input->PushKey(DIK_LSHIFT))
			velocity.y -= moveSpeed; // 下降

		// 座標に加算
		worldTransform.translation_.x += velocity.x;
		worldTransform.translation_.y += velocity.y;
		worldTransform.translation_.z += velocity.z;

		// デバッグ中もカメラ行列の更新が必要
		railCamera_.Update();

	} else {
		// ▼▼▼ ゲーム本編モード（レール移動） ▼▼▼

		// ★既存の switch文 をここにそのまま入れます
		switch (phase_) {
		case Phase::kWait:
			timer_ += 1.0f / 60.0f;
			if (timer_ >= 2.0f) {
				phase_ = Phase::kMove;
				splineT_ = 0.0f;
			}
			break;

		case Phase::kIntro:
			phase_ = Phase::kMove;
			break;

		case Phase::kMove: {
			// ----------------------------------------------------
			// 0. スプライン進行 (既存のまま)
			// ----------------------------------------------------
			splineT_ += moveSpeed_;
			if (splineT_ >= 1.0f) {
				splineT_ = 1.0f;
				phase_ = Phase::kEnd;
			}

			// ----------------------------------------------------
			// 1. キー入力による移動 (WASD)
			// ----------------------------------------------------
			Vector3 moveInput = {0, 0, 0};
			float playerSpeed = 0.3f; // プレイヤーの移動速度

			if (input->PushKey(DIK_W))
				moveInput.y += playerSpeed;
			if (input->PushKey(DIK_S))
				moveInput.y -= playerSpeed;
			if (input->PushKey(DIK_D))
				moveInput.x += playerSpeed;
			if (input->PushKey(DIK_A))
				moveInput.x -= playerSpeed;

			// ローカル座標に加算
			playerLocalPos_.x += moveInput.x;
			playerLocalPos_.y += moveInput.y;

			// ★移動制限（画面外に出ないように制限）
			// 画面サイズに合わせて数値は調整してください
			const float kLimitX = 14.0f;
			const float kLimitY = 9.0f;
			playerLocalPos_.x = std::clamp(playerLocalPos_.x, -kLimitX, kLimitX);
			playerLocalPos_.y = std::clamp(playerLocalPos_.y, -kLimitY, kLimitY);

			// ----------------------------------------------------
			// 2. レール上の座標計算 (カメラ用)
			// ----------------------------------------------------
			Vector3 railPos = CatmullRomSpline(splineControlPoints_, splineT_);

			// カメラの位置更新
			worldTransform.translation_.x = railPos.x;
			worldTransform.translation_.y = railPos.y;
			// worldTransform.translation_.z = railPos.z; // Z移動なし設定のまま

			// カメラ回転は固定（酔い防止）
			worldTransform.rotation_ = {0.0f, 0.0f, 0.0f};

			// ----------------------------------------------------
			// 3. 自機の座標計算 (レール位置 + WASD操作分)
			// ----------------------------------------------------

			// ★ここが重要：レールの位置に、操作した分(LocalPos)を足す
			playerWorldTransform_.translation_.x = worldTransform.translation_.x + playerLocalPos_.x;
			playerWorldTransform_.translation_.y = worldTransform.translation_.y + playerLocalPos_.y;

			// Z座標：カメラより少し前(15.0f)に固定する
			// ※もし奥に進ませたいなら railPos.z を使いますが、今回は固定でいきます
			playerWorldTransform_.translation_.z = worldTransform.translation_.z + 15.0f;

			// ----------------------------------------------------
			// 4. 自機の回転制御 (レール挙動 + 入力連動)
			// ----------------------------------------------------

			// レールの未来位置を取得してカーブを予測
			float lookAheadT = splineT_ + 0.002f;
			if (lookAheadT > 1.0f)
				lookAheadT = 1.0f;
			Vector3 nextRailPos = CatmullRomSpline(splineControlPoints_, lookAheadT);

			// レール自体の移動量
			float railVelX = nextRailPos.x - railPos.x;
			float railVelY = nextRailPos.y - railPos.y;

			// 各種感度
			float bankStrength = 0.1f;      // レールのカーブに対する傾き
			float inputBankStrength = 0.2f; // キー入力に対する傾き(クイッと動く感じ)

			// 目標とする角度を計算
			// 「レールのカーブ(-railVelX)」 と 「自分の操作(-moveInput.x)」 の両方を反映
			float targetRotZ = -(railVelX * bankStrength) - (moveInput.x * inputBankStrength);
			float targetRotX = -(railVelY * bankStrength) - (moveInput.y * inputBankStrength);

			// 制限 (最大45度 = 約0.8ラジアン)
			playerWorldTransform_.rotation_.z = std::clamp(targetRotZ, -0.8f, 0.8f);
			playerWorldTransform_.rotation_.x = std::clamp(targetRotX, -0.8f, 0.8f);

			// 少し旋回（ヨー）を入れると自然になります
			playerWorldTransform_.rotation_.y = (moveInput.x * 0.1f);

			// ----------------------------------------------------
			// 5. 行列更新
			// ----------------------------------------------------
			playerWorldTransform_.matWorld_ = MakeAffineMatrix(playerWorldTransform_.scale_, playerWorldTransform_.rotation_, playerWorldTransform_.translation_);
			playerWorldTransform_.TransferMatrix();

			// プレイヤー本体へ反映
			player_.GetWorldTransform().translation_ = playerWorldTransform_.translation_;
			player_.GetWorldTransform().rotation_ = playerWorldTransform_.rotation_;

			railCamera_.Update();
			break;
		}
		case Phase::kEnd:
			break;
		}
	}

	// ==================================================
	// 3. 共通更新処理
	// ==================================================

	// 天球
	skydome_.Update();

	// プレイヤー
	player_.Update();

	// 地面（海）の位置をカメラに合わせる
	ground_.Update(railCamera_.GetWorldTransform().matWorld_);

	// エネミー群更新
	if (enemy_)
		enemy_->Update();
	if (enemyAimer_)
		enemyAimer_->Update();
	if (enemyHoming_)
		enemyHoming_->Update();

	// 当たり判定
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
