#include "EnemyManager.h"
#include "MyMath.h"

using namespace KamataEngine;

EnemyManager::~EnemyManager() {
	for (auto* e : enemies_)
		delete e;
	for (auto* e : aimers_)
		delete e;
	for (auto* e : homings_)
		delete e;
	for (auto* e : follows_)
		delete e;
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

	normalBulletModel_ = Model::CreateFromOBJ("enemyBullet");
	if (!normalBulletModel_)
		normalBulletModel_ = Model::Create();
	homingBulletModel_ = Model::CreateFromOBJ("homingBullet");
	if (!homingBulletModel_)
		homingBulletModel_ = Model::Create();

	LoadEnemyData();
	timer_ = 0.0f;
}

// 全滅判定の実装
bool EnemyManager::IsAllFollowEnemiesDead() const {
	// 1. 今画面にいる Follow 敵が残っているなら false
	if (!follows_.empty()) {
		return false;
	}

	// 2. まだ出現していないスポーンデータの中に Follow (Type 3) があるなら false
	for (const auto& data : spawnList_) {
		if (data.type == 3) {
			return false;
		}
	}

	// 両方なければ全滅
	return true;
}

void EnemyManager::LoadEnemyData() {
	// Type 3 = Follow (カメラ相対固定)

	std::string csvText = R"(
		# --- WAVE 1: 序盤 ---
		1.0, 0,  -5, 0, 40
		1.5, 0,   5, 0, 40
		2.0, 0,   0, 3, 40

		# --- WAVE 2: Followお試し (左上と右上に常駐) ---
		4.0, 3,  -6,  4, 25
		4.5, 3,   6,  4, 25

		# --- WAVE 3: 自機狙いとの連携 ---
		7.0, 1, -10, 5, 60
		7.5, 1,  10, 5, 60
		8.0, 3,   0, -5, 30
		
		# --- WAVE 4: Follow増量 (囲み) ---
		12.0, 3, -10,  6, 30
		12.0, 3,  10,  6, 30
		12.2, 3,  -5,  3, 25
		12.2, 3,   5,  3, 25

		# --- WAVE 5: Homingとの波状攻撃 ---
		16.0, 2,  -8, 5, 60
		16.5, 2,   8, 5, 60
		17.0, 3,   0, 8, 30
		17.5, 3,   0, -5, 30

		# --- WAVE 6: 上下からの挟み撃ちFollow ---
		22.0, 3,  -8,  8, 25
		22.0, 3,   8,  8, 25
		22.5, 3,  -8, -8, 25
		22.5, 3,   8, -8, 25
		
		# --- WAVE 7: 縦一列 ---
		26.0, 3,   6,  6, 30
		26.2, 3,   6,  3, 30
		26.4, 3,   6,  0, 30
		26.6, 3,   6, -3, 30
		26.8, 3,   6, -6, 30
		
		# --- WAVE 8: ラッシュ ---
		30.0, 0,  -5, 0, 40
		30.2, 0,   5, 0, 40
		30.4, 1,   0, 10, 50
		31.0, 3,  -8, 0, 20
		31.0, 3,   8, 0, 20
		32.0, 2,   0, 0, 60
	)";

	std::stringstream ss(csvText);
	std::string line;
	spawnList_.clear();
	while (std::getline(ss, line)) {
		if (line.empty() || line.length() < 5 || line[0] == '#' || (line.find_first_not_of(" \t") != std::string::npos && line[line.find_first_not_of(" \t")] == '#'))
			continue;

		std::stringstream lineSs(line);
		std::string segment;
		std::vector<std::string> segs;
		while (std::getline(lineSs, segment, ',')) {
			segs.push_back(segment);
		}
		if (segs.size() >= 5) {
			EnemySpawnData data;
			data.time = std::stof(segs[0]);
			data.type = std::stoi(segs[1]);
			data.position.x = std::stof(segs[2]);
			data.position.y = std::stof(segs[3]);
			data.position.z = std::stof(segs[4]);
			spawnList_.push_back(data);
		}
	}
	spawnList_.sort([](const EnemySpawnData& a, const EnemySpawnData& b) { return a.time < b.time; });
}

void EnemyManager::Update(const Matrix4x4& cameraMat, const Vector3& cameraRot) {
	timer_ += 1.0f / 60.0f;

	while (!spawnList_.empty()) {
		const auto& data = spawnList_.front();
		if (data.time > timer_)
			break;

		Vector3 spawnWorldPos = Transform(data.position, cameraMat);

		if (data.type == 0) { // Enemy
			Enemy* newEnemy = new Enemy();
			newEnemy->Initialize(enemyModel_);
			newEnemy->SetPosition(spawnWorldPos);
			newEnemy->SetRotation(cameraRot);
			Vector3 localVel = {0, 0, -0.2f};
			Vector3 worldVel = TransformNormal(localVel, cameraMat);
			newEnemy->SetVelocity(worldVel);
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

		} else if (data.type == 3) { // Follow
			EnemyFollow* newFollow = new EnemyFollow();
			newFollow->Initialize(enemyFollowModel_, player_, data.position);
			follows_.push_back(newFollow);
		}
		spawnList_.pop_front();
	}

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

	for (auto it = follows_.begin(); it != follows_.end();) {
		(*it)->Update(cameraMat);
		if ((*it)->IsDead()) {
			delete *it;
			it = follows_.erase(it);
		} else {
			++it;
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
}