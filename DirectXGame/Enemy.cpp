#include "Enemy.h"
#include "MapChipField.h"
#include "MyMath.h"
#include "Player.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 初期は左向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
	facingDir_ = -1;

	// デフォルトは左へ歩行
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	walkTimer = 0.0f;

	// 飛行タイプ用の基準高さ
	baseY_ = position.y;
}

bool Enemy::IsGroundBelow(const Vector3& pos) const {
	if (!map_)
		return false;
	// 足元ちょい下を調べる
	Vector3 probe = pos;
	probe.y -= (kHeight * 0.5f + 0.05f);
	auto idx = map_->GetMapChipIndexSetByPosition(probe);
	return map_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex) == MapChipType::kBlock;
}

bool Enemy::IsLedgeAhead(int dir) const {
	if (!map_)
		return false;
	// 進行方向の足元を調べる
	Vector3 probe = worldTransform_.translation_;
	probe.x += dir * (kWidth * 0.5f + 0.25f);
	probe.y -= (kHeight * 0.5f + 0.05f);
	auto idx = map_->GetMapChipIndexSetByPosition(probe);
	return map_->GetMapChipTypeByIndex(idx.xIndex, idx.yIndex) != MapChipType::kBlock;
}

void Enemy::Update() {
	// 歩行見た目のアニメ（上下揺れ等）
	walkTimer += 1.0f / 60.0f;

	aiTimer_ += 1.0f / 60.0f;

	Vector3 pos = worldTransform_.translation_;

	// ----------------
	// タイプ別AI更新
	// ----------------
	switch (type_) {
	case Type::Walker: {
		// 足場の端で反転
		int dir = (velocity_.x >= 0.0f) ? +1 : -1;
		if (IsLedgeAhead(dir))
			velocity_.x = -velocity_.x;

		// 重力（下向き＝マイナスY）
		if (!IsGroundBelow(pos)) {
			velocity_.y -= kGrav;
			velocity_.y = max(velocity_.y, -kFallVMax);
		} else {
			velocity_.y = 0.0f;
		}
		break;
	}
	case Type::Jumper: {
		if (IsGroundBelow(pos)) {
			// 周期ジャンプ
			if (aiTimer_ >= 1.2f) {
				velocity_.y = kJumpVy; // 上向き正
				aiTimer_ = 0.0f;
			}
			int dir = (velocity_.x >= 0.0f) ? +1 : -1;
			if (IsLedgeAhead(dir))
				velocity_.x = -velocity_.x;
		}
		// 空中は重力
		velocity_.y -= kGrav;
		velocity_.y = max(velocity_.y, -kFallVMax);
		break;
	}
	case Type::Chaser: {
		// 一定距離内ならプレイヤーへ接近
		if (target_) {
			Vector3 tp = target_->GetWorldPosition();
			float dx = tp.x - pos.x;
			float dy = tp.y - pos.y;
			float dist2 = dx * dx + dy * dy;
			if (dist2 <= kChaseRange * kChaseRange) {
				velocity_.x = (dx >= 0.0f ? +1.0f : -1.0f) * kChaseSpeed;
			} else {
				// 徘徊：端で反転
				int dir = (velocity_.x >= 0.0f) ? +1 : -1;
				if (IsLedgeAhead(dir))
					velocity_.x = -velocity_.x;
			}
		}
		// 重力
		if (!IsGroundBelow(pos)) {
			velocity_.y -= kGrav;
			velocity_.y = max(velocity_.y, -kFallVMax);
		} else {
			velocity_.y = 0.0f;
		}
		break;
	}
	case Type::Flyer: {
		// ふわふわ飛行（重力なし）
		float phase = walkTimer * (2.0f * std::numbers::pi_v<float>)*kFlyHz;
		pos.y = baseY_ + std::sin(phase) * kFlyAmp;
		worldTransform_.translation_.y = pos.y;
		velocity_.y = 0.0f; // 飛行は重力なし
		break;
	}
	}

	// 位置更新（Flyer は Y を上書き済みなので X が主に加算される）
	worldTransform_.translation_ += velocity_;

	// ----------------
	// 滑らか振り向き制御（全タイプ共通）
	// ----------------
	// 進行方向から望ましい向きを決定（±x が十分ある時のみ）
	int desiredDir = facingDir_;
	if (velocity_.x > kFaceEps)
		desiredDir = +1;
	else if (velocity_.x < -kFaceEps)
		desiredDir = -1;

	// 望ましい向きに変化があれば、回転アニメ開始
	if (desiredDir != facingDir_) {
		facingDir_ = desiredDir;
		turnFirstRotationY_ = worldTransform_.rotation_.y;
		turnTimer_ = kTimeTurn;
	}

	// 目的角（右=π/2, 左=3π/2）
	float destinationRotationYTable[] = {
	    std::numbers::pi_v<float> / 2.0f,       // index 0: right（使わない）
	    std::numbers::pi_v<float> / 2.0f,       // +1 → right
	    std::numbers::pi_v<float> * 3.0f / 2.0f // -1 → left
	};
	float destinationRotationY = (facingDir_ >= 0) ? destinationRotationYTable[1] : destinationRotationYTable[2];

	// タイマーが動いている間は EaseInOut で補間（プレイヤーと同じ方式：カウントダウン）
	if (turnTimer_ > 0.0f) {
		turnTimer_ = max(turnTimer_ - (1.0f / 60.0f), 0.0f);
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	} else {
		// 静止時は正確に目的角へ
		worldTransform_.rotation_.y = destinationRotationY;
	}

	// 見た目の歩行アニメ（上下揺れ）
	worldTransform_.rotation_.x = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);

	

	// 行列更新
	WorldTransformUpdate(worldTransform_);
	// --- ここから追加：マップ外で即死 ---
	if (map_) {
		const float left = 0.0f;
		const float right = (map_->GetNumBlockHorizontal() - 1) * MapChipField::kBlockWidth;
		const float bottom = 0.0f;
		const float top = (map_->GetNumBlockVirtical() - 1) * MapChipField::kBlockHeight;

		const Vector3 worldPos = GetWorldPosition(); // ← ここをリネーム
		const float margin = 0.5f;

		if (worldPos.x < left - margin || worldPos.x > right + margin || worldPos.y < bottom - margin || worldPos.y > top + margin) {
			isDead_ = true; // Kill()/OnDamageでもOK
			return;
		}
	}
}

void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }

AABB Enemy::GetAABB() {
	Vector3 p = GetWorldPosition();
	AABB aabb;
	aabb.min = {p.x - kWidth * 0.5f, p.y - kHeight * 0.5f, p.z - kWidth * 0.5f};
	aabb.max = {p.x + kWidth * 0.5f, p.y + kHeight * 0.5f, p.z + kWidth * 0.5f};
	return aabb;
}

Vector3 Enemy::GetWorldPosition() {
	Vector3 p;
	p.x = worldTransform_.matWorld_.m[3][0];
	p.y = worldTransform_.matWorld_.m[3][1];
	p.z = worldTransform_.matWorld_.m[3][2];
	return p;
}

void Enemy::OnCollision(const Player* player) {
	(void)player;
	// 必要があれば個別の応答を実装
}

void Enemy::OnDamage(float damage, const Vector3& fromDir) {
	if (isDead_)
		return;
	hp_ -= damage;
	// 簡易ノックバック
	velocity_.x += (fromDir.x >= 0.0f ? +1.0f : -1.0f) * 0.15f;
	velocity_.y = 0.12f;

	if (hp_ <= 0.0f) {
		isDead_ = true;
		// 必要ならここでエフェクト生成やSE再生をトリガー
	}
}
