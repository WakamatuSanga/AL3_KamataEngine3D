#include "EnemyManager.h"
#include "MyMath.h"

using namespace KamataEngine;

void EnemyManager::Initialize(Player* player) {
	player_ = player;

	// モデルロード
	enemyModel_ = Model::CreateFromOBJ("enemy");
	enemyAimerModel_ = Model::CreateFromOBJ("enemy");
	enemyHomingModel_ = Model::CreateFromOBJ("enemy");

	normalBulletModel_ = Model::CreateFromOBJ("enemyBullet");
	homingBulletModel_ = Model::CreateFromOBJ("homingBullet");

	// モデル読み込み失敗時のフォールバック
	if (!enemyModel_)
		enemyModel_ = Model::Create();
	if (!enemyAimerModel_)
		enemyAimerModel_ = Model::Create();
	if (!enemyHomingModel_)
		enemyHomingModel_ = Model::Create();
	if (!normalBulletModel_)
		normalBulletModel_ = Model::Create();
	if (!homingBulletModel_)
		homingBulletModel_ = Model::Create();

	LoadEnemyData();
	timer_ = 0.0f;
}

void EnemyManager::LoadEnemyData() {
	// フォーマット: "発生時間, 敵タイプ(0-2), X, Y, Z"
	std::string csvText = R"(
		1.0, 0,  -5, 0, 40
		1.5, 0,   5, 0, 40
		2.0, 0,   0, 3, 40
		
		3.0, 1, -10, 2, 50
		3.5, 1,  10, 2, 50
		
		5.0, 2,  -8, 5, 60
		5.5, 2,   8, 5, 60
		6.0, 2,   0, 8, 60
		
		8.0, 0,  -5, 0, 40
		8.2, 0,  -3, 0, 40
		8.4, 0,  -1, 0, 40
		8.6, 0,   1, 0, 40
		8.8, 0,   3, 0, 40
	)";

	std::stringstream ss(csvText);
	std::string line;

	spawnList_.clear();

	while (std::getline(ss, line)) {
		if (line.empty() || line.length() < 5)
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

void EnemyManager::Update() {
	timer_ += 1.0f / 60.0f;

	// --- スポーン処理 ---
	while (!spawnList_.empty()) {
		const auto& data = spawnList_.front();
		if (data.time > timer_)
			break;

		if (data.type == 0) { // Enemy
			Enemy* newEnemy = new Enemy();
			newEnemy->Initialize(enemyModel_, normalBulletModel_);
			newEnemy->SetPosition(data.position);
			enemies_.push_back(newEnemy);

		} else if (data.type == 1) { // Aimer
			EnemyAimer* newAimer = new EnemyAimer();
			newAimer->Initialize(enemyAimerModel_, normalBulletModel_, player_);
			newAimer->SetPosition(data.position);
			aimers_.push_back(newAimer);

		} else if (data.type == 2) { // Homing
			EnemyHoming* newHoming = new EnemyHoming();
			newHoming->Initialize(enemyHomingModel_, homingBulletModel_, player_);
			newHoming->SetPosition(data.position);
			homings_.push_back(newHoming);
		}
		spawnList_.pop_front();
	}

	// --- 更新と死亡削除 ---
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
}

void EnemyManager::Draw(Camera& camera) {
	for (auto* e : enemies_)
		e->Draw(camera);
	for (auto* e : aimers_)
		e->Draw(camera);
	for (auto* e : homings_)
		e->Draw(camera);
}

EnemyManager::~EnemyManager() {
	for (auto* e : enemies_)
		delete e;
	for (auto* e : aimers_)
		delete e;
	for (auto* e : homings_)
		delete e;
	enemies_.clear();
	aimers_.clear();
	homings_.clear();

	delete enemyModel_;
	delete enemyAimerModel_;
	delete enemyHomingModel_;
	delete normalBulletModel_;
	delete homingBulletModel_;
}