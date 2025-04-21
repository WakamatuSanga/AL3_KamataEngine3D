#include "Player.h"
#include"KamataEngine.h"
#include"cassert"
using namespace KamataEngine;

void Player::Initialize(Model* model, uint32_t textureHandle,Camera* camera) { 
	assert(model); 

	 textureHandle_ = textureHandle;
	
	 model_ = model;

	 camera_=camera;


	worldTransform_.Initialize();
}

void Player::Update() {

	worldTransform_.TransferMatrix();

}

void Player::Draw() { 
	model_->Draw(worldTransform_, *camera_, textureHandle_); 
}
