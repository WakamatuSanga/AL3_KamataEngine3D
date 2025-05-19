
#include "Player.h"
#include "MyMath.h"
#include "cassert"

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	camera_ = camera;
}

void Player::Update() {
	for (std::vector<KamataEngine::WorldTransform*>& worldTransformPlayerLine : worldTransformPlayer_) {
		for (KamataEngine::WorldTransform* worldTransformPlayer : worldTransformPlayerLine) {
			if (!worldTransformPlayer) {
				continue;
			}
			worldTransformPlayer->matWorld_ = MakeAffineMatrix(worldTransformPlayer->scale_, worldTransformPlayer->rotation_, worldTransformPlayer->translation_);
			worldTransformPlayer->TransferMatrix();
		}
	}
}

void Player::Draw() { model_->Draw(worldTransform_, *camera_); }
