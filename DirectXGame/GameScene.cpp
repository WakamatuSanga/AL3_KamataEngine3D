#include "GameScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"
#include "MyMath.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete modelSkydome_;
	delete groundModel_;
	delete playerModel_;
	delete cloudModel_;
	delete enemyManager_;
}

void GameScene::Initialize() {
	camera_.Initialize();
	camera_.UpdateMatrix();
	PrimitiveDrawer::GetInstance()->Initialize();

	railCamera_.Initialize({0, 0.0f, -15.0f}, {0, 0, 0}, 0.45f, 0.1f, 5000.0f);
	playerWorldTransform_.Initialize();

	phase_ = Phase::kWait;
	splineT_ = 0.0f;
	timer_ = 0.0f;
	playerLocalPos_ = {0, 0, 0};
	isDebugCamera_ = false;

	// コース定義
	splineControlPoints_ = {
	    {0.0f, 0.0f, -50.0f  },
        {0.0f, 0.0f, 0.0f    },

	    {0.0f, 0.0f, 1000.0f },
        {0.0f, 0.0f, 2000.0f },
        {0.0f, 0.0f, 3000.0f },
        {0.0f, 0.0f, 4000.0f },
        {0.0f, 0.0f, 5000.0f },
	    {0.0f, 0.0f, 6000.0f },
        {0.0f, 0.0f, 7000.0f },
        {0.0f, 0.0f, 8000.0f },
        {0.0f, 0.0f, 9000.0f },
        {0.0f, 0.0f, 10000.0f},

	    {0.0f, 0.0f, 10050.0f},
        {0.0f, 0.0f, 10100.0f}
    };

	splinePoints_.clear();
	const size_t segmentCount = 1000;
	for (size_t i = 0; i <= segmentCount; i++) {
		float t = (float)i / segmentCount;
		Vector3 pos = CatmullRomSpline(splineControlPoints_, t);
		splinePoints_.push_back(pos);
	}

	groundModel_ = Model::CreateFromOBJ("sea", true);
	ground_.Initialize(groundModel_);

	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	if (!modelSkydome_)
		modelSkydome_ = Model::Create();
	skydome_.Initialize(modelSkydome_);

	playerModel_ = Model::CreateFromOBJ("player");
	player_.Initialize(playerModel_);

	cloudModel_ = Model::CreateFromOBJ("cloud");
	if (!cloudModel_) {
		cloudModel_ = Model::Create();
	}
	clouds_.Initialize(cloudModel_, 50);

	enemyManager_ = new EnemyManager();
	enemyManager_->Initialize(&player_);
}

void GameScene::Update() {
	auto* input = Input::GetInstance();

	if (player_.IsDead()) {
		SceneManager::GetInstance()->ChangeScene(new GameOverScene());
		return;
	}

	if (input->TriggerKey(DIK_0)) {
		isDebugCamera_ = !isDebugCamera_;
		if (isDebugCamera_) {
			debugCameraRot_ = railCamera_.GetWorldTransform().rotation_;
			preMousePos_ = input->GetMousePosition();
		}
	}

	WorldTransform& camWT = railCamera_.GetWorldTransform();

	if (isDebugCamera_) {
		Vector2 mousePos = input->GetMousePosition();
		if (input->IsPressMouse(0)) {
			float moveX = mousePos.x - preMousePos_.x;
			float moveY = mousePos.y - preMousePos_.y;
			const float sensitivity = 0.01f;
			debugCameraRot_.y += moveX * sensitivity;
			debugCameraRot_.x += moveY * sensitivity;
		}
		preMousePos_ = mousePos;
		camWT.rotation_ = debugCameraRot_;

		const float moveSpeed = 1.0f;
		Vector3 velocity = {0, 0, 0};
		Matrix4x4 matRotY = MakeRotateYMatrix(camWT.rotation_.y);

		if (input->PushKey(DIK_UP))
			velocity.z += moveSpeed;
		if (input->PushKey(DIK_DOWN))
			velocity.z -= moveSpeed;
		if (input->PushKey(DIK_LEFT))
			velocity.x -= moveSpeed;
		if (input->PushKey(DIK_RIGHT))
			velocity.x += moveSpeed;

		velocity = TransformNormal(velocity, matRotY);
		if (input->PushKey(DIK_SPACE))
			velocity.y += moveSpeed;
		if (input->PushKey(DIK_LSHIFT))
			velocity.y -= moveSpeed;

		camWT.translation_ += velocity;
		railCamera_.Update();

	} else {
		// --- メイン更新 ---
		Vector3 currentRailPos = {0, 0, 0};
		Vector3 nextRailPos = {0, 0, 1.0f};
		Vector3 prevRailPos = {0, 0, 0};

		switch (phase_) {
		case Phase::kWait:
			timer_ += 1.0f / 60.0f;
			currentRailPos = CatmullRomSpline(splineControlPoints_, 0.0f);
			prevRailPos = currentRailPos;
			nextRailPos = CatmullRomSpline(splineControlPoints_, 0.01f);
			if (timer_ >= 2.0f) {
				phase_ = Phase::kMove;
				splineT_ = 0.0f;
			}
			break;

		case Phase::kIntro:
			phase_ = Phase::kMove;
			break;

		case Phase::kMove: {
			float prevT = splineT_;
			if (splineT_ < 1.0f) {
				splineT_ += moveSpeed_;
				if (splineT_ >= 1.0f) {
					splineT_ = 1.0f;
				}
			}

			currentRailPos = CatmullRomSpline(splineControlPoints_, splineT_);
			prevRailPos = CatmullRomSpline(splineControlPoints_, prevT);

			float lookAheadT = min(splineT_ + 0.005f, 1.0f);
			nextRailPos = CatmullRomSpline(splineControlPoints_, lookAheadT);

			// ボスが撃破されたらクリアへ遷移
			if (enemyManager_ && enemyManager_->IsBossDead()) {
				phase_ = Phase::kEnd;
			}
			break;
		}
		case Phase::kEnd:
			SceneManager::GetInstance()->ChangeScene(new ClearScene());
			return;
		}

		// プレイヤー操作
		Vector3 moveInput = {0, 0, 0};
		float playerSpeed = 0.3f;
		if (input->PushKey(DIK_W))
			moveInput.y += playerSpeed;
		if (input->PushKey(DIK_S))
			moveInput.y -= playerSpeed;
		if (input->PushKey(DIK_D))
			moveInput.x += playerSpeed;
		if (input->PushKey(DIK_A))
			moveInput.x -= playerSpeed;

		playerLocalPos_.x += moveInput.x;
		playerLocalPos_.y += moveInput.y;

		const float kLimitX = 9.0f;
		const float kLimitY = 5.5f;
		playerLocalPos_.x = std::clamp(playerLocalPos_.x, -kLimitX, kLimitX);
		playerLocalPos_.y = std::clamp(playerLocalPos_.y, -kLimitY, kLimitY);

		playerWorldTransform_.translation_.x = playerLocalPos_.x;
		playerWorldTransform_.translation_.y = playerLocalPos_.y;
		playerWorldTransform_.translation_.z = 0.0f;

		Vector3 railDir = nextRailPos - currentRailPos;
		float railLen = Length(railDir);
		if (railLen > 0.0f)
			railDir = railDir / railLen;

		float bankStrength = 0.1f;
		float inputBankStrength = 0.2f;
		float targetRotZ = -(railDir.x * bankStrength) - (moveInput.x * inputBankStrength);
		float targetRotX = -(railDir.y * bankStrength) - (moveInput.y * inputBankStrength);

		playerWorldTransform_.rotation_.z = std::clamp(targetRotZ, -0.8f, 0.8f);
		playerWorldTransform_.rotation_.x = std::clamp(targetRotX, -0.8f, 0.8f);
		playerWorldTransform_.rotation_.y = (moveInput.x * 0.1f);

		playerWorldTransform_.matWorld_ = MakeAffineMatrix(playerWorldTransform_.scale_, playerWorldTransform_.rotation_, playerWorldTransform_.translation_);
		playerWorldTransform_.TransferMatrix();

		player_.GetWorldTransform().scale_ = playerWorldTransform_.scale_;
		player_.GetWorldTransform().rotation_ = playerWorldTransform_.rotation_;
		player_.GetWorldTransform().translation_ = playerWorldTransform_.translation_;

		railCamera_.Update();

		float railYaw = std::atan2(railDir.x, railDir.z);
		float lenXZ = std::sqrt(railDir.x * railDir.x + railDir.z * railDir.z);
		float railPitch = std::atan2(-railDir.y, lenXZ);

		skydome_.SetPosition({0.0f, 0.0f, 0.0f});
		skydome_.SetRotation({-railPitch, -railYaw, 0.0f});

		Vector3 groundPos = {0.0f, -520.0f, 0.0f};
		groundPos.x = currentRailPos.x * -1.0f;
		groundPos.y = -520.0f + (currentRailPos.y * -1.0f);
		ground_.SetPosition(groundPos);

		Vector3 moveDiff = currentRailPos - prevRailPos;
		float moveDist = Length(moveDiff);
		float rotSpeedX = moveDist / 520.0f;
		ground_.Update(rotSpeedX);

		clouds_.Update(railCamera_.GetWorldTransform().translation_);

		if (enemyManager_) {
			enemyManager_->Update(railCamera_.GetWorldTransform().matWorld_, railCamera_.GetWorldTransform().rotation_);
		}
	}

	skydome_.Update();

	// レティクル用更新
	player_.Update(railCamera_.GetCamera());

	CheckAllCollisions();
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	
	Model::PreDraw(dxCommon->GetCommandList());

	Camera& cam = railCamera_.GetCamera();
	player_.Draw(cam);
	if (enemyManager_)
		enemyManager_->Draw(cam);

	skydome_.Draw(cam);
	ground_.Draw(cam);
	clouds_.Draw(cam);
	
	Sprite::PreDraw(dxCommon->GetCommandList());
	player_.DrawUI();
	Sprite::PostDraw();

	Model::PostDraw();
}

void GameScene::CheckAllCollisions() {
	if (!enemyManager_)
		return;

	auto distSq = [](const Vector3& a, const Vector3& b) {
		float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
		return dx * dx + dy * dy + dz * dz;
	};

	Vector3 playerPos = player_.GetPosition();
	float rPlayer = player_.GetCollisionRadius();
	const auto& playerBullets = player_.GetBullets();

	auto collidePlayerVsBullets = [&](const auto& bullets) {
		for (auto* b : bullets) {
			if (!b || b->IsDead())
				continue;
			float r = rPlayer + b->GetCollisionRadius();
			if (distSq(playerPos, b->GetPosition()) <= r * r) {
				player_.OnCollision();
				b->OnCollision();
			}
		}
	};

	auto collidePBulletVsEnemy = [&](const Vector3& ePos, float eRad, auto* enemy) {
		for (auto* pb : playerBullets) {
			if (!pb || pb->IsDead())
				continue;
			float r = eRad + pb->GetCollisionRadius();
			if (distSq(ePos, pb->GetPosition()) <= r * r) {
				pb->OnCollision();
				if (enemy)
					enemy->OnCollision();
			}
		}
	};

	auto collidePBulletVsEBullet = [&](const auto& eBullets) {
		for (auto* pb : playerBullets) {
			if (!pb || pb->IsDead())
				continue;
			for (auto* eb : eBullets) {
				if (!eb || eb->IsDead())
					continue;
				float r = pb->GetCollisionRadius() + eb->GetCollisionRadius();
				if (distSq(pb->GetPosition(), eb->GetPosition()) <= r * r) {
					pb->OnCollision();
					eb->OnCollision();
				}
			}
		}
	};

	// 雑魚敵
	for (auto* e : enemyManager_->GetEnemies()) {
		collidePlayerVsBullets(e->GetBullets());
		collidePBulletVsEnemy(e->GetPosition(), e->GetCollisionRadius(), e);
		collidePBulletVsEBullet(e->GetBullets());
	}
	for (auto* e : enemyManager_->GetAimers()) {
		collidePlayerVsBullets(e->GetBullets());
		collidePBulletVsEnemy(e->GetPosition(), e->GetCollisionRadius(), e);
		collidePBulletVsEBullet(e->GetBullets());
	}
	for (auto* e : enemyManager_->GetHomings()) {
		collidePlayerVsBullets(e->GetBullets());
		collidePBulletVsEnemy(e->GetPosition(), e->GetCollisionRadius(), e);
		collidePBulletVsEBullet(e->GetBullets());
	}
	for (auto* e : enemyManager_->GetFollows()) {
		collidePlayerVsBullets(e->GetBullets());
		collidePBulletVsEnemy(e->GetPosition(), e->GetCollisionRadius(), e);
		collidePBulletVsEBullet(e->GetBullets());
	}

	// ボスの当たり判定
	EnemyBoss* boss = enemyManager_->GetBoss();
	if (boss && !boss->IsDead()) {
		collidePlayerVsBullets(boss->GetBullets());
		collidePlayerVsBullets(boss->GetHomingBullets());
		collidePBulletVsEnemy(boss->GetPosition(), boss->GetCollisionRadius(), boss);
		collidePBulletVsEBullet(boss->GetBullets());
		collidePBulletVsEBullet(boss->GetHomingBullets());
	}
}