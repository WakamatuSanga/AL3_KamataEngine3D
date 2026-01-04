#include "EnemyManager.h"
#include "MyMath.h"

using namespace KamataEngine;

EnemyManager::~EnemyManager() {
	// リスト内の敵を削除
	for (auto* e : enemies_)
		delete e;
	for (auto* e : aimers_)
		delete e;
	for (auto* e : homings_)
		delete e;
	enemies_.clear();
	aimers_.clear();
	homings_.clear();

	// モデルの削除
	delete enemyModel_;
	delete enemyAimerModel_;
	delete enemyHomingModel_;
	delete normalBulletModel_;
	delete homingBulletModel_;
}

void EnemyManager::Initialize(Player* player) {
	player_ = player;

	// モデル読み込み
	enemyModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyModel_)
		enemyModel_ = Model::Create();

	enemyAimerModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyAimerModel_)
		enemyAimerModel_ = Model::Create();

	enemyHomingModel_ = Model::CreateFromOBJ("enemy");
	if (!enemyHomingModel_)
		enemyHomingModel_ = Model::Create();

	normalBulletModel_ = Model::CreateFromOBJ("enemyBullet");
	if (!normalBulletModel_)
		normalBulletModel_ = Model::Create();

	homingBulletModel_ = Model::CreateFromOBJ("homingBullet");
	if (!homingBulletModel_)
		homingBulletModel_ = Model::Create();

	// 敵発生データの読み込み
	LoadEnemyData();
	timer_ = 0.0f;
}

void EnemyManager::LoadEnemyData() {
	// CSV形式の文字列データ (時間, タイプ, x, y, z)
	// type: 0=通常, 1=自機狙い, 2=ホーミング
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
			newEnemy->Initialize(enemyModel_);
			// ★ SetPosition が実装されたのでコメントアウト解除
			newEnemy->SetPosition(data.position);
			enemies_.push_back(newEnemy);

		} else if (data.type == 1) { // Aimer
			EnemyAimer* newAimer = new EnemyAimer();
			newAimer->Initialize(enemyAimerModel_, player_);
			// ★ SetPosition が実装されたのでコメントアウト解除
			newAimer->SetPosition(data.position);
			aimers_.push_back(newAimer);

		} else if (data.type == 2) { // Homing
			EnemyHoming* newHoming = new EnemyHoming();
			newHoming->Initialize(enemyHomingModel_, player_);
			// ★ SetPosition が実装されたのでコメントアウト解除
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