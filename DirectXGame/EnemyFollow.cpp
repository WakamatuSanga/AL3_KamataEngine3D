#include "EnemyFollow.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

EnemyFollow::~EnemyFollow() {
	for (auto* b : bullets_)
		delete b;
	bullets_.clear();
}

void EnemyFollow::Initialize(Model* model, Player* player, const Vector3& offset) {
	model_ = model;
	player_ = player;
	offset_ = offset;
	isDead_ = false;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.2f, 1.2f, 1.2f}; // 少し大きめ
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	// 初期位置は計算前なので一旦原点
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	bulletModel_ = Model::CreateFromOBJ("enemyBullet");
	if (!bulletModel_)
		bulletModel_ = Model::Create();

	shotTimer_ = 0;
	lifeTimer_ = 0;
	appearTimer_ = 0.0f;
}

float EnemyFollow::GetCollisionRadius() const {
	return 1.2f; // 固定サイズ
}

void EnemyFollow::FireAimedBullet() {
	if (!bulletModel_ || !player_)
		return;

	const float kBulletSpeed = 0.4f; // 追従中は弾速遅めでいやらしく
	Vector3 myPos = worldTransform_.translation_;
	Vector3 playerPos = player_->GetPosition();

	Vector3 dir = playerPos;
	dir -= myPos;
	dir = Normalized(dir);

	Vector3 vel = dir * kBulletSpeed;

	auto* b = new EnemyBullet();
	b->Initialize(bulletModel_, myPos, vel);
	b->SetScale(0.5f);
	b->SetLifeTime(180); // 長めに残る
	bullets_.push_back(b);
}

void EnemyFollow::Update(const Matrix4x4& cameraMat) {
	if (isDead_)
		return;

	// 1. 出現演出 (0.0 -> 1.0)
	// 最初は画面奥からフワッと定位置に来る感じに
	if (appearTimer_ < 1.0f) {
		appearTimer_ += 0.02f;
		if (appearTimer_ > 1.0f)
			appearTimer_ = 1.0f;
	}
	float t = EaseOut(0.0f, 1.0f, appearTimer_);

	// 2. カメラ行列を使って「あるべき場所」を計算
	//    目標位置 = カメラ座標 + (カメラの向き * offset)
	Vector3 targetPos = Transform(offset_, cameraMat);

	//    出現演出中は「さらに奥(+Z)」から手前へ来るように補間
	//    カメラ空間でのZ+50くらいからスッと入ってくる
	Vector3 startOffset = offset_;
	startOffset.z += 50.0f;
	Vector3 startPos = Transform(startOffset, cameraMat);

	worldTransform_.translation_ = Lerp(startPos, targetPos, t);

	// 3. ふわふわ浮遊させる（Y軸）
	float floatY = std::sinf((float)lifeTimer_ * 0.05f) * 0.5f;
	// カメラの上方向ベクトルを取得して、その方向にずらす
	Vector3 cameraUp = {cameraMat.m[0][1], cameraMat.m[1][1], cameraMat.m[2][1]};
	worldTransform_.translation_ += cameraUp * floatY;

	// 4. プレイヤーの方を向く（ビルボード的な処理、あるいはY軸回転のみ）
	if (player_) {
		Vector3 p = player_->GetPosition();
		Vector3 d = p - worldTransform_.translation_;
		worldTransform_.rotation_.y = std::atan2(d.x, d.z);
		// 上下も向かせるとより自然
		float len = std::sqrt(d.x * d.x + d.z * d.z);
		worldTransform_.rotation_.x = std::atan2(-d.y, len);
	}

	// 5. 射撃
	if (appearTimer_ >= 1.0f) { // 定位置に着いてから攻撃開始
		++shotTimer_;
		if (shotTimer_ >= shotInterval_) {
			shotTimer_ = 0;
			FireAimedBullet();
		}
	}

	// 6. 寿命
	++lifeTimer_;
	if (lifeTimer_ > lifeTime_) {
		// 時間切れで退場（奥へ飛び去るなど）
		// ここでは簡易的に死亡
		isDead_ = true;
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 弾更新
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		(*it)->Update();
		if ((*it)->IsDead()) {
			delete *it;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}
}

void EnemyFollow::Draw(Camera& camera) {
	if (isDead_)
		return;
	if (model_)
		model_->Draw(worldTransform_, camera);
	for (auto* b : bullets_)
		b->Draw(camera);
}