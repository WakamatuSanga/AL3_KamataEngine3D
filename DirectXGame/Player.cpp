#define NOMINMAX

#include "Player.h"
#include "MapChipField.h"
#include "MyMath.h"
#include "cassert"
#include <algorithm>
#include <numbers>
#include "MapChipField.h"
using namespace KamataEngine;
using namespace MathUtility;



void Player::Initialize(Model* model, Camera* camera, Vector3& position) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	camera_ = camera;
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info) {

	// 上昇あり
	if (info.move.y<=0) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));

	}

	MapChipType mapChipTyoe;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipTyoe = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipTyoe==MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipTyoe = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipTyoe == MapChipType::kBlock) {
		hit = true;
	}
	// ブロックにヒット
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(0, +kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y=std::max(0.0f,rect.bottom-worldTransform_.translation_.y-(kHeight/2.0f+kBlank));
		// 天井衝突フラグ
		info.ceiling = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {

	if (info.move.y >= 0) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info) {

	if (info.move.x <= 0) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {

	if (info.move.x >= 0) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
}

// マップ衝突判定
void Player::CheckMapCollision(CollisionMapInfo& info) { 

	CheckMapCollisionUp(info); 
	CheckMapCollisionDown(info); 
	CheckMapCollisionRight(info); 
	CheckMapCollisionLeft(info); 



}


Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
        {-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
        {+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}  // kLeftTop
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}
void Player::MapHitMove(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_ += info.move;

}
void Player::CheckMapCeiling(const CollisionMapInfo& info) {
	// 天井に当たった
	if (info.ceiling) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}

}
void Player::Update() {

	//worldTransform_.translation_ += velocity_;



	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();

	// 移動入力
	InputMove();

	// 移動量を加味して衝突判定する

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;
	// マップ衝突チェック
	CheckMapCollision(collisionMapInfo);

	MapHitMove(collisionMapInfo);

	CheckMapCeiling(collisionMapInfo);

	AnimateTurn();

	bool landing = false;

	

	if (velocity_.y < 0) {
		if (worldTransform_.translation_.y <= 1.0f) {
			landing = true;
		}
	}

	if (landing) {
		// めり込み
		worldTransform_.translation_.y = 1.0f;
		// 摩擦
		velocity_.x *= (1.0f - kAttenuation);
		velocity_.y = 0.0f;
		onGround_ = true;
	}
}


// 移動入力
void Player::InputMove() {

	// 左右移動操作
	if (onGround_) {
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;
				if (lrDirection_ != LRDirection::kRiget) {
					lrDirection_ = LRDirection::kRiget;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}

			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ += acceleration;

			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_ += Vector3(0, kJumpAcceleration, 0);
			if (velocity_.y > 0.0f) {
				onGround_ = false;
			}
		}

	} else // 空中
	{
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
		
	}
}

void Player::AnimateTurn() {

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}
}



void Player::Draw() { model_->Draw(worldTransform_, *camera_); }


