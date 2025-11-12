#include "GameScene.h"
#include "MyMath.h" // MakeAffineMatrix など、行列/ベクトルユーティリティ
#include <cmath>
using namespace KamataEngine;


//  GameScene::Initialize
//  シーン開始時に一度だけ呼ばれる初期化。カメラ/スプライン/プレイヤーを作る。

void GameScene::Initialize() {
	// --- カメラ初期化（投影・ビュー用の内部バッファなどを初期化） ---
	camera_.Initialize();

	// --- ステージ用スプライン制御点の定義 ---
	// Catmull-Rom の安定化のため、前後に「番兵（sentinel）」を置くのが定石。
	// ここでは Z+ 方向に進む緩い S 字カーブを作って、レール移動の雰囲気を先に確認する。
	std::vector<Vector3> cp = {
	    {-2, 0, -10}, // [0] 先頭番兵（実際の走行区間外）
	    {0,  0, 0  }, // [1] 走行開始点
	    {1,  0, 10 }, // [2]
	    {-1, 0, 20 }, // [3]
	    {2,  0, 30 }, // [4]
	    {0,  0, 40 }, // [5]
	    {0,  0, 50 }, // [6] 走行終端
	    {0,  0, 55 }  // [7] 終端番兵（実際の走行区間外）
	};
	spline_.SetControlPoints(cp);

	// --- レールカメラの初期化 ---
	// スプラインに沿って前進し、曲率に応じた「バンク（機体や視点の傾き）」を付ける。
	railCam_.Initialize(&spline_, &camera_);
	railCam_.SetSpeedPerFrame(0.02f);  // 1フレームあたりの t 進行量（＝ステージの進み具合）
	railCam_.SetFollowDist(7.5f);      // カメラの追従距離（視点の位置を進行点の少し後ろに置く）
	railCam_.SetLookAhead(4.0f);       // 注視点の先読み距離（視点が少し先を向く）
	railCam_.SetBankParam(0.9f, 0.6f); // バンク強度k と 最大角（ラジアン）

	// --- プレイヤーの仮モデル（見た目） ---
	// 最初は「cube.obj」でOK。後で専用機体モデルに差し替えれば良い。
	modelPlayer_ = Model::CreateFromOBJ("cube");

	// --- プレイヤー初期化（移動だけ先に実装。射撃は Step 2 で追加） ---
	player_.Initialize(modelPlayer_);

	// カメラから前方8mの平面に乗せ、やや下寄せ（-0.55）
	player_.SetViewPlaneDist(8.0f);
	player_.SetScreenBiasY(-0.55f);


	// --- 敵の仮モデル---
	modelEnemy_ = Model::CreateFromOBJ("cube");
	enemyManager_.Initialize(modelEnemy_);

	// --- デバッグカメラ（0キーで切替） ---
	// シーンの見回し・当たり判定の目視確認などに便利。リリース時には無効化でOK。
	debugCamera_ = new DebugCamera(1280, 720);
	// （補足）
	// worldTransformBlocks_ 等のステージ配置テストは、必要になった段階で戻して使ってください。
}

// 球同士の当たり判定（距離の2乗で比較）
static bool SphereHit(const KamataEngine::Vector3& aPos, float aR, const KamataEngine::Vector3& bPos, float bR) {
	using namespace KamataEngine;
	Vector3 d = {aPos.x - bPos.x, aPos.y - bPos.y, aPos.z - bPos.z};
	float dist2 = d.x * d.x + d.y * d.y + d.z * d.z;
	float r = aR + bR;
	return dist2 <= (r * r);
}

void GameScene::CheckCollisionPlayerBulletsVsEnemies() {
	const auto& bullets = player_.GetBullets();
	const auto& enemies = enemyManager_.GetEnemies();

	for (Enemy* e : enemies) {
		if (!e || e->IsDead())
			continue;

		for (Bullet* b : bullets) {
			if (!b || b->IsDead())
				continue;

			if (SphereHit(e->GetPosition(), e->GetRadius(), b->GetPosition(), b->GetRadius())) {
				// ヒット！
				e->Damage(1);          // とりあえず1ダメージで即死
				player_.KillBullet(b); // 弾は消す

				// 同じ弾で複数の敵に当たらないように break
				break;
			}
		}
	}
}

//  GameScene::Update
//  毎フレーム呼ばれる更新処理。入力/カメラ/プレイヤーなどのロジックを進める。

void GameScene::Update() {
#ifdef _DEBUG
	// --- 0キー：デバッグカメラとゲームカメラの切替 ---
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	if (isDebugCameraActive_) {
		// --- デバッグカメラ有効時：自由視点で見る ---
		debugCamera_->Update();

		// DebugCamera が持つ Camera を直接 Game 用 Camera に反映
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		// --- ゲームカメラ（レールカメラ）での自動前進 ---
		// スプラインに沿って視点が進み、曲率に応じて軽く傾く（バンク）。
		railCam_.Update();
	}
	const float dt = 1.0f / 60.0f;
	// --- プレイヤー移動（画面内 XY） ---
	// レールの「スクリーン平面」を基準に、W/A/S/D で自機を移動。
	// A/D ダブルタップで「見た目のロール演出」（無敵・弾消しは Step 3 で付与予定）。
	player_.Update(railCam_);

	// 敵の更新（カメラ基準・奥から手前へ）
	enemyManager_.Update(railCam_, dt);

	CheckCollisionPlayerBulletsVsEnemies();
}


//  GameScene::Draw
//  毎フレーム呼ばれる描画処理。2D（UI）→3D（モデル）の順に描く。

void GameScene::Draw() {
	// --- DirectX コマンドリストの取得 ---
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//==================== 2D描画（スプライト/UI） ====================
	Sprite::PreDraw(dxCommon->GetCommandList());

	// ここに HP/スコア/チェイン/チャージゲージ等の UI を描く予定（Step 2 で追加）。

	Sprite::PostDraw();

	//==================== 3D描画（モデル） ============================
	Model::PreDraw(dxCommon->GetCommandList());

	// --- プレイヤー機体の描画 ---
	player_.Draw(camera_);
	// --- 敵機体の描画 ---
	enemyManager_.Draw(camera_);

	// 将来的には：敵/弾/背景オブジェクト/リング/障害物などもここで描画していく。

	Model::PostDraw();
}


//  GameScene::~GameScene
//  シーン終了時の後片付け。new したものを解放。

GameScene::~GameScene() {
	// --- もしグリッド状のブロック等を new していれば安全に解放 ---
	for (auto& line : worldTransformBlocks_) {
		for (auto* wt : line) {
			delete wt;
		}
	}
	worldTransformBlocks_.clear();

	// --- Sprite / 汎用 Model は生成していなければ nullptr のままなので delete 安全 ---
	delete spreite_;
	delete model_;

	// --- デバッグカメラ解放 ---
	delete debugCamera_;

	// --- 機体モデルの所有関係に注意 ---
	// KamataEngine 側でリソース管理（キャッシュ/参照カウント）している場合は delete 不要。
	// もし明示的 delete が必要な設計なら、ここで delete modelShip_; を呼んでください。
	// delete modelShip_;
}
