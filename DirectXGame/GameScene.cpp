#include "GameScene.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	// カメラ
	camera_.Initialize();
	// プレイヤーモデル（OBJ名はプロジェクトに合わせて）
	playerModel_ = Model::CreateFromOBJ("player"); // 無ければ "cube" など
	player_.Initialize(playerModel_);

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
	camera_.UpdateMatrix();
	// 天球
	skydome_.Update();
	// プレイヤー
	player_.Update();
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

	// プレイヤー
	player_.Draw(camera_);

	// エネミー群
	if (enemy_) {
		enemy_->Draw(camera_);
	}
	if (enemyAimer_) {
		enemyAimer_->Draw(camera_);
	}
	if (enemyHoming_) {
		enemyHoming_->Draw(camera_);
	}
	// 天球
	skydome_.Draw(camera_);
	Model::PostDraw();
}

GameScene::~GameScene() {

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete enemy_;
	enemy_ = nullptr;

	delete enemyModel_;
	enemyModel_ = nullptr;

	delete enemyAimer_;
	enemyAimer_ = nullptr;

	delete enemyAimerModel_;
	enemyAimerModel_ = nullptr;

	delete enemyHomingModel_;
	enemyHomingModel_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;
}
