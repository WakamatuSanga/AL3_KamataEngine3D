#include "EnemyManager.h"
#include "MyMath.h"
#include <cmath> // std::sin, std::cos
#include <fstream>
#include <iostream>

using namespace KamataEngine;

// 乱数ヘルパー（配置をばらつかせる用）
static float RandF(float minVal, float maxVal) {
	float r = (float)rand() / RAND_MAX;
	return minVal + r * (maxVal - minVal);
}

EnemyManager::~EnemyManager() {
	for (auto* e : enemies_)
		delete e;
	for (auto* e : aimers_)
		delete e;
	for (auto* e : homings_)
		delete e;
	for (auto* e : follows_)
		delete e;
	if (boss_)
		delete boss_;

	enemies_.clear();
	aimers_.clear();
	homings_.clear();
	follows_.clear();

	delete enemyModel_;
	delete enemyAimerModel_;
	delete enemyHomingModel_;
	delete enemyFollowModel_;
	delete normalBulletModel_;
	delete homingBulletModel_;
}

void EnemyManager::Initialize(Player* player) {
	player_ = player;

	// モデル生成
	enemyModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyModel_)
		enemyModel_ = Model::Create();

	enemyAimerModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyAimerModel_)
		enemyAimerModel_ = Model::Create();

	enemyHomingModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyHomingModel_)
		enemyHomingModel_ = Model::Create();

	enemyFollowModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyFollowModel_)
		enemyFollowModel_ = Model::Create();

	// 弾モデル
	normalBulletModel_ = Model::CreateFromOBJ("enemyBullet");
	if (!normalBulletModel_)
		normalBulletModel_ = Model::Create();

	homingBulletModel_ = Model::CreateFromOBJ("homingBullet");
	if (!homingBulletModel_)
		homingBulletModel_ = Model::Create();

	LoadEnemyData();
	timer_ = 0.0f;

	boss_ = nullptr;
}

void EnemyManager::Update(const Matrix4x4& cameraMat, const Vector3& cameraRot) {
	timer_ += 1.0f / 60.0f;

	// スポーン処理
	while (!spawnList_.empty()) {
		const auto& data = spawnList_.front();
		if (data.time > timer_) {
			break;
		}

		Vector3 spawnWorldPos = data.position;

		// 雑魚敵はカメラのZ位置を足して奥に出現させる
		if (data.type != 4) {
			spawnWorldPos.z += cameraMat.m[3][2];
		}

		if (data.type == 0) { // Normal
			Enemy* newEnemy = new Enemy();
			newEnemy->Initialize(enemyModel_);
			newEnemy->SetPosition(spawnWorldPos);
			newEnemy->SetRotation(cameraRot);
			enemies_.push_back(newEnemy);

		} else if (data.type == 1) { // Aimer
			EnemyAimer* newAimer = new EnemyAimer();
			newAimer->Initialize(enemyAimerModel_, player_);
			newAimer->SetPosition(spawnWorldPos);
			newAimer->SetRotation(cameraRot);
			aimers_.push_back(newAimer);

		} else if (data.type == 2) { // Homing
			EnemyHoming* newHoming = new EnemyHoming();
			newHoming->Initialize(enemyHomingModel_, player_);
			newHoming->SetPosition(spawnWorldPos);
			newHoming->SetRotation(cameraRot);
			homings_.push_back(newHoming);

		} else if (data.type == 3) { // Follow (フェアリー)
			EnemyFollow* newFollow = new EnemyFollow();
			// Followはカメラからの相対位置で動く
			newFollow->Initialize(enemyFollowModel_, player_, data.position);
			follows_.push_back(newFollow);

		} else if (data.type == 4) { // ★ Boss
			if (!boss_) {
				boss_ = new EnemyBoss();
				boss_->Initialize(enemyModel_, normalBulletModel_, homingBulletModel_, player_);
			}
		}

		spawnList_.pop_front();
	}

	// 各敵更新
	auto updateAndClean = [](auto& vec) {
		for (auto it = vec.begin(); it != vec.end();) {
			(*it)->Update();
			if ((*it)->IsDead()) {
				delete *it;
				it = vec.erase(it);
			} else {
				++it;
			}
		}
	};

	updateAndClean(enemies_);
	updateAndClean(aimers_);
	updateAndClean(homings_);

	// Followはカメラ行列
	for (auto it = follows_.begin(); it != follows_.end();) {
		(*it)->Update(cameraMat);
		if ((*it)->IsDead()) {
			delete *it;
			it = follows_.erase(it);
		} else {
			++it;
		}
	}

	// ボス更新
	if (boss_) {
		Vector3 camPos = {cameraMat.m[3][0], cameraMat.m[3][1], cameraMat.m[3][2]};
		boss_->Update(camPos);

		if (boss_->IsDead()) {
			delete boss_;
			boss_ = nullptr;
		}
	}
}

void EnemyManager::Draw(Camera& camera) {
	for (auto* e : enemies_)
		e->Draw(camera);
	for (auto* e : aimers_)
		e->Draw(camera);
	for (auto* e : homings_)
		e->Draw(camera);
	for (auto* e : follows_)
		e->Draw(camera);

	if (boss_) {
		boss_->Draw(camera);
	}
}

void EnemyManager::LoadEnemyData() {
	// ★調整方針：
	// 1. Z位置を40～50と非常に近くして圧迫感を出す
	// 2. X,Y座標を中央(-10～10程度)に密集させる
	// 3. 敵の数を増やし、フェアリー(Type 3)を早期投入

	// --- Wave 1: 開幕ラッシュ (2.0s ~ ) ---
	// 通常敵を中央に縦列配置 (数珠つなぎ)
	for (int i = 0; i < 5; ++i) {
		float z = 40.0f + i * 5.0f; // 40, 45, 50... 手前から奥へ
		spawnList_.push_back({
		    2.0f + i * 0.3f, 0, {10.0f, 0.0f, z}
        });
	}
	// 早速フェアリーを左右に配置 (プレイヤーの近くにまとわりつく)
	/*spawnList_.push_back({
	    3.0f, 3, {-8.0f, 2.0f, 15.0f}
    });
	spawnList_.push_back({
	    3.0f, 3, {8.0f, 2.0f, 15.0f}
    });*/

	// --- Wave 2: 密集編隊 (5.0s ~ ) ---
	// 狙い撃ち(Aimer)を中央付近に密集させる
	spawnList_.push_back({
	    5.0f, 1, {-5.0f, 5.0f, 50.0f}
    });
	spawnList_.push_back({
	    5.0f, 1, {5.0f, 5.0f, 50.0f}
    });
	spawnList_.push_back({
	    5.5f, 1, {-5.0f, -5.0f, 50.0f}
    });
	spawnList_.push_back({
	    5.5f, 1, {5.0f, -5.0f, 50.0f}
    });
	// 中央にフェアリー追加
	spawnList_.push_back({
	    6.0f, 3, {0.0f, 5.0f, 40.0f}
    });
	spawnList_.push_back({
	    6.5f, 3, {0.0f, -5.0f, 40.0f}
    });

	// --- Wave 3: ホーミング＆フェアリー包囲網 (10.0s ~ ) ---
	// ホーミング敵を円形に配置（中央を取り囲む）
	int numHoming = 8;
	for (int i = 0; i < numHoming; ++i) {
		float angle = (float)i * (6.28f / numHoming);
		float r = 10.0f; // 半径10と狭く
		float x = std::cos(angle) * r;
		float y = std::sin(angle) * r;
		spawnList_.push_back({
		    10.0f + i * 0.2f, 2, {x, y, 60.0f}
        });
	}
	//// フェアリーも追加
	//spawnList_.push_back({
	//    11.0f, 3, {-12.0f, 0.0f, 25.0f}
 //   });
	//spawnList_.push_back({
	//    11.5f, 3, {12.0f, 0.0f, 25.0f}
 //   });

	// --- Wave 4: 総力戦 (15.0s ~ ) ---
	// 画面中央から大量の雑魚敵が湧き出る
	for (int i = 0; i < 15; ++i) {
		float t = 15.0f + i * 0.3f;
		// ランダムに少し散らすが、基本は中央
		float x = RandF(-8.0f, 8.0f);
		float y = RandF(-6.0f, 6.0f);
		spawnList_.push_back({
		    t, 0, {x, y, 40.0f}
        });
	}
	// 合間にAimerとFollowを混ぜる
	spawnList_.push_back({
	    16.0f, 1, {-15.0f, 5.0f, 55.0f}
    });
	spawnList_.push_back({
	    17.0f, 1, {15.0f, -5.0f, 55.0f}
    });
	spawnList_.push_back({
	    18.0f, 3, {0.0f, 5.0f, 30.0f}
    });

	// --- Boss Battle (25.0s ~ ) ---
	spawnList_.push_back({
	    25.0f, 4, {0.0f, 0.0f, 0.0f}
    });
}

bool EnemyManager::IsAllFollowEnemiesDead() const {
	for (const auto& data : spawnList_) {
		if (data.type == 3)
			return false;
	}
	if (!follows_.empty())
		return false;
	return true;
}

bool EnemyManager::IsBossDead() const {
	for (const auto& data : spawnList_) {
		if (data.type == 4)
			return false;
	}
	if (boss_)
		return false;
	return true;
}

void EnemyManager::DrawUI() {
	if (boss_) {
		boss_->DrawUI();
	}
}
bool EnemyManager::GetReticleTarget(const Vector2& mousePos, const Matrix4x4& matVPV, Vector3& hitPos) const {
	bool isHit = false;
	float minDepth = 1.0f;

	// ★修正：固定の判定半径を使う (150.0f)
	// 敵のモデルサイズに関わらず、画面上で「レティクルの中心から150ピクセル以内」にあればロックする
	// これにより「近くても遠くても、レティクルに乗ればロックする」挙動になります。
	const float fixedLockRadius = 150.0f;
	const float lockRadiusSq = fixedLockRadius * fixedLockRadius;

	auto checkHit = [&](const Vector3& enemyPos) {
		Vector3 targetPos = enemyPos;

		Vector3 screenPos = Transform(targetPos, matVPV);

		// 画面外（前後）の敵は除外
		// ★手前すぎると計算がおかしくなるので、0.0f付近も除外
		if (screenPos.z < 0.01f || screenPos.z > 1.0f)
			return;

		// 画面上の距離をチェック
		float dx = screenPos.x - mousePos.x;
		float dy = screenPos.y - mousePos.y;
		float distSq = dx * dx + dy * dy;

		// 固定半径で判定
		if (distSq <= lockRadiusSq) {
			if (screenPos.z < minDepth) {
				minDepth = screenPos.z;
				hitPos = targetPos;
				isHit = true;
			}
		}
	};

	for (const auto* e : enemies_)
		checkHit(e->GetPosition());
	for (const auto* e : aimers_)
		checkHit(e->GetPosition());
	for (const auto* e : homings_)
		checkHit(e->GetPosition());
	for (const auto* e : follows_)
		checkHit(e->GetPosition());

	if (boss_ && !boss_->IsDead()) {
		checkHit(boss_->GetPosition());
	}

	return isHit;
}