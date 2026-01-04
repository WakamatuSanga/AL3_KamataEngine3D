#include "GameScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

GameScene::~GameScene() {
	delete modelSkydome_;
	delete groundModel_;
	delete playerModel_;
	delete enemyManager_;
}

void GameScene::Initialize() {
	camera_.Initialize();
	camera_.UpdateMatrix();
	PrimitiveDrawer::GetInstance()->Initialize();
	railCamera_.Initialize({0, 2.0f, -10.0f}, {0, 0, 0}, 0.45f, 0.1f, 5000.0f);
	playerWorldTransform_.Initialize();

	phase_ = Phase::kWait;
	splineT_ = 0.0f;
	timer_ = 0.0f;
	playerLocalPos_ = {0, 0, 0};
	isDebugCamera_ = false;

	// コース定義
	splineControlPoints_ = {
	    {0.0f,    0.0f,   -50.0f },
        {0.0f,    0.0f,   0.0f   },
        {0.0f,    0.0f,   200.0f },
        {200.0f,  150.0f, 600.0f },
        {-300.0f, 40.0f,  1000.0f},
        {300.0f,  350.0f, 1500.0f},
	    {-150.0f, 300.0f, 1800.0f},
        {200.0f,  40.0f,  2200.0f},
        {-200.0f, 150.0f, 2600.0f},
        {0.0f,    0.0f,   3000.0f},
        {0.0f,    0.0f,   3100.0f},
        {0.0f,    0.0f,   3200.0f}
    };

	splinePoints_.clear();
	const size_t segmentCount = 500;
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

	// ★ EnemyManager 初期化
	enemyManager_ = new EnemyManager();
	enemyManager_->Initialize(&player_);
}

void GameScene::Update() {
	auto* input = Input::GetInstance();

	if (player_.IsDead()) {
		SceneManager::GetInstance()->ChangeScene(new GameOverScene());
		return;
	}

	// --- デバッグカメラ処理 ---
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
			splineT_ += moveSpeed_;
			if (splineT_ >= 1.0f) {
				splineT_ = 1.0f;
				phase_ = Phase::kEnd;
			}

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

			const float kLimitX = 14.0f;
			const float kLimitY = 9.0f;
			playerLocalPos_.x = std::clamp(playerLocalPos_.x, -kLimitX, kLimitX);
			playerLocalPos_.y = std::clamp(playerLocalPos_.y, -kLimitY, kLimitY);

			Vector3 railPos = CatmullRomSpline(splineControlPoints_, splineT_);

			camWT.translation_.x = railPos.x;
			camWT.translation_.y = railPos.y;
			camWT.rotation_ = {0, 0, 0};

			playerWorldTransform_.translation_.x = camWT.translation_.x + playerLocalPos_.x;
			playerWorldTransform_.translation_.y = camWT.translation_.y + playerLocalPos_.y;
			playerWorldTransform_.translation_.z = camWT.translation_.z + 15.0f;

			float lookAheadT = min(splineT_ + 0.002f, 1.0f);
			Vector3 nextRailPos = CatmullRomSpline(splineControlPoints_, lookAheadT);
			float railVelX = nextRailPos.x - railPos.x;
			float railVelY = nextRailPos.y - railPos.y;

			float bankStrength = 0.1f;
			float inputBankStrength = 0.2f;
			float targetRotZ = -(railVelX * bankStrength) - (moveInput.x * inputBankStrength);
			float targetRotX = -(railVelY * bankStrength) - (moveInput.y * inputBankStrength);

			playerWorldTransform_.rotation_.z = std::clamp(targetRotZ, -0.8f, 0.8f);
			playerWorldTransform_.rotation_.x = std::clamp(targetRotX, -0.8f, 0.8f);
			playerWorldTransform_.rotation_.y = (moveInput.x * 0.1f);

			playerWorldTransform_.matWorld_ = MakeAffineMatrix(playerWorldTransform_.scale_, playerWorldTransform_.rotation_, playerWorldTransform_.translation_);
			playerWorldTransform_.TransferMatrix();

			player_.GetWorldTransform().translation_ = playerWorldTransform_.translation_;
			player_.GetWorldTransform().rotation_ = playerWorldTransform_.rotation_;

			railCamera_.Update();
			break;
		}
		case Phase::kEnd:
			SceneManager::GetInstance()->ChangeScene(new ClearScene());
			break;
		}
	}

	skydome_.Update();
	player_.Update();
	ground_.Update(railCamera_.GetWorldTransform().matWorld_);

	// ★ EnemyManager のみ更新
	if (enemyManager_)
		enemyManager_->Update();

	CheckAllCollisions();
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();
	Model::PreDraw(dxCommon->GetCommandList());

	Camera& cam = railCamera_.GetCamera();
	player_.Draw(cam);
	if (enemyManager_)
		enemyManager_->Draw(cam);
	skydome_.Draw(cam);
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
					enemy->OnCollision(); // 敵側の被弾処理
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

	// EnemyManager から取得して判定
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
}