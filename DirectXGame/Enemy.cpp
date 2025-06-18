
#define NOMINMAX

#include "Enemy.h"
#include "MapChipField.h"
#include "MyMath.h"
#include "cassert"
#include <algorithm>
#include <numbers>


using namespace KamataEngine;
using namespace MathUtility;



void Enemy::Initialize(Model* model, Camera* camera, Vector3& position) {

	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	camera_ = camera;
}

void Enemy::Update() {}

void Enemy::Draw() {}
