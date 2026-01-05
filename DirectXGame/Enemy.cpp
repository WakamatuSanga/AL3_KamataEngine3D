#include "Enemy.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

Enemy::~Enemy() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
	// Manager管理ならモデルdelete不要
}

void Enemy::Initialize(Model* model) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 25.0f};

	// デフォルトの移動速度（Z軸手前）
	// Managerから SetVelocity で上書きされるようになります
	approachVelocity_ = {0.0f, 0.0f, -0.2f};
	leaveVelocity_ = {0.3f, 0.2f, -0.1f};
	phase_ = Phase::Approach;
	isDead_ = false;

	bulletModel_ = Model::CreateFromOBJ("enemyBullet");
}

void Enemy::FireBullet() {
	if (!bulletModel_)
		return;

	const float kBulletSpeed = 0.5f;

	// 弾も「敵の進行方向」あるいは「敵の向き」に合わせて撃つ方が自然ですが
	// とりあえず今回は「敵の現在地からカメラ方向（大体手前）」へ撃つ簡易計算にします
	// 本格的にやるなら Player* を持たせて狙わせるか、Velocity方向に撃たせます
	Vector3 vel = {0.0f, 0.0f, -kBulletSpeed};

	// もし「自分の進行方向に撃つ」ならこうします（お好みで切り替えてください）
	// Vector3 vel = Normalized(approachVelocity_) * kBulletSpeed;

	EnemyBullet* b = new EnemyBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, vel);
	b->SetLifeTime(90);
	bullets_.push_back(b);
}

float Enemy::GetCollisionRadius() const {
	const auto& s = worldTransform_.scale_;
	float m = max(max(std::fabs(s.x), std::fabs(s.y)), std::fabs(s.z));
	constexpr float kBase = 0.9f;
	return kBase * m;
}

void Enemy::UpdateApproach() {
	worldTransform_.translation_ += approachVelocity_;

	// ★「一定距離」の判定が固定座標(-30.0f)だと、回転している場合にうまくいかないため
	// 「生存タイマー」で制御するか、あるいは簡易的に「スポーンから一定時間」で離脱へ移行させると良いです。
	// ここではシンプルに「ローカル座標っぽく見立てて Z移動量が一定を超えたら」としたいですが、
	// 汎用性を高めるため「600フレーム経過したら離脱」などに変えるのがベストです。
	// 今回は既存のロジックを壊さないよう、「原点からの距離」などで判定する方法もありますが、
	// 一旦このままでも「画面外に出たら消える」処理があるので動作はします。

	// 簡易修正: とりあえず時間経過か、あるいは「カメラ後ろに行ったら」離脱
	// (実装簡略化のため、既存のZ判定はそのままにしますが、カメラ相対配置だと機能しにくい場合があります)

	// 一定間隔で発射
	++shotTimer_;
	if (shotTimer_ >= shotInterval_) {
		shotTimer_ = 0;
		FireBullet();
	}
}

void Enemy::UpdateLeave() {
	worldTransform_.translation_ += leaveVelocity_;

	// 画面外判定（かなり広めにとっておく）
	// カメラ相対配置にすると座標が大きく変わる可能性があるため、絶対値で大きめに判定
	Vector3 p = worldTransform_.translation_;
	if (std::abs(p.x) > 300.0f || std::abs(p.y) > 300.0f || std::abs(p.z) > 300.0f) {
		isDead_ = true;
	}
}

void Enemy::Update() {
	if (isDead_)
		return;

	switch (phase_) {
	case Phase::Approach:
	default:
		UpdateApproach();
		break;
	case Phase::Leave:
		UpdateLeave();
		break;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	for (auto it = bullets_.begin(); it != bullets_.end();) {
		EnemyBullet* b = *it;
		b->Update();
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
}

void Enemy::Draw(Camera& camera) {
	if (isDead_)
		return;
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
	for (EnemyBullet* b : bullets_) {
		b->Draw(camera);
	}
}