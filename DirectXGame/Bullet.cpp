#include "Bullet.h"
#include "MyMath.h"

using namespace KamataEngine;

void Bullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	model_ = model;
	velocity_ = velocity;
	life_ = 2.0f; // 2秒生きる（お好みで）

	radius_ = 0.4f; // 当たり半径（小さめ）

	world_.Initialize();
	world_.translation_ = position;
	world_.rotation_ = {0, 0, 0};
	world_.scale_ = {0.2f, 0.2f, 0.2f};
	world_.matWorld_ = MakeAffineMatrix(world_.scale_, world_.rotation_, world_.translation_);
	world_.TransferMatrix();
}

void Bullet::Update(float dt) {
	if (life_ <= 0.0f)
		return;

	life_ -= dt;
	if (life_ <= 0.0f)
		return;

	world_.translation_.x += velocity_.x * dt;
	world_.translation_.y += velocity_.y * dt;
	world_.translation_.z += velocity_.z * dt;

	world_.matWorld_ = MakeAffineMatrix(world_.scale_, world_.rotation_, world_.translation_);
	world_.TransferMatrix();
}

void Bullet::Draw(Camera& camera) {
	if (life_ <= 0.0f || !model_)
		return;
	model_->Draw(world_, camera);
}
