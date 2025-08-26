#define NOMINMAX

#include "Player.h"
#include "MapChipField.h"
#include "MyMath.h"

#include <algorithm>
#include <cassert>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model);
	// モデル
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	camera_ = camera;
	// 色オブジェクト（点滅用）初期化
	playerColor_.Initialize();

	hp_ = maxHp_ = 3; // 初期HP
}

// 移動入力(02_07 スライド10枚目)
void Player::InputMove() {

	if (onGround_) {

		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {

				if (velocity_.x < 0.0f) {
					// 旋回の最初は移動減衰をかける
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f) {
					// 旋回の最初は移動減衰をかける
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ += acceleration;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ほぼ0の場合に0にする
		if (std::abs(velocity_.x) <= 0.0001f) {
			velocity_.x = 0.0f;
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ += Vector3(0, kJumpAcceleration / 60.0f, 0);
		}
	} else {
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration / 60.0f, 0);
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

// 02_07 スライド13枚目
void Player::CheckMapCollision(CollisionMapInfo& info) {

	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

// 02_07 スライド14枚目(上下左右全て)
void Player::CheckMapCollisionUp(CollisionMapInfo& info) {

	// 02_07スライド20枚目 上昇あり?
	if (info.move.y <= 0) {
		return;
	}

	// 02_07 スライド19枚目（下のfor文も）
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 02_07 スライド28枚目（下のfor文も）
	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？ 02_07 スライド34枚目
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			info.ceiling = true;
		}
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {

	info;

	// 02_08 スライド7枚目 下降あり？
	if (info.move.y >= 0) {
		return;
	}

	// 02_08 スライド7枚目（下のfor文も）
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 02_08 スライド8枚目(右下、左下の判定まで)
	MapChipType mapChipType;

	// フラグ初期化
	bool hit = false;

	// 左下の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 02_08スライド11枚目 ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		// 地面に当たったことを記録する
		info.landing = true;
	}
}

// 02_08スライド14枚目 設置状態の切り替え処理
void Player::UpdateOnGround(const CollisionMapInfo& info) {

	info;

	if (onGround_) {
		// 02_08スライド18枚目 ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 落下判定
			// 落下なら空中状態に切り替え

			// 02_08スライド19枚目(このelseブロック全部)
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}

			bool hit = false;

			MapChipType mapChipType;

			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) {
				//				DebugText::GetInstance()->ConsolePrintf("jump");
				onGround_ = false;
			}
		}
	} else {
		// 02_08スライド16枚目 地面に接触している場合の処理
		if (info.landing) {
			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロに
			velocity_.y = 0.0f;
		}
	}
}

// 02_08スライド27枚目 壁接地中の処理
void Player::UpdateOnWall(const CollisionMapInfo& info) {

	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::TakeDamage(int amount) {
	if (IsInvincible() || isDead_)
		return;

	hp_ = std::max(0, hp_ - amount);
	if (hp_ <= 0) {
		isDead_ = true; // 既存のフェーズ遷移（Death→Fade）に繋がる
	} else {
		StartInvincible(kInvincibleDuration); // 既存の無敵＆点滅へ
	}
}

// 中身入れるのは02_08スライド25枚目
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {

	if (info.move.x <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 右側の当たり判定
	bool hit = false;

	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(+kWidth / 2.0f, 0, 0));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

// 中身入れるのは02_08スライド25枚目
void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {

	if (info.move.x >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 右側の当たり判定
	bool hit = false;

	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::max(0.0f, rect.right - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
		}
	}
}

// 02_07 スライド17枚目
Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, //  kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  //  kLeftTop
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}



void Player ::Update() {

	// 移動入力(02_07 スライド10枚目)
	InputMove();

	 // 先に攻撃入力を見る
	TryStartAttack();

	// 既存の移動入力
	InputMove();

	// 衝突情報を初期化(02_07 スライド13枚目)
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	// マップ衝突チェック(02_07 スライド13枚目)
	CheckMapCollision(collisionMapInfo);

	// 移動(02_07 スライド36枚目)
	worldTransform_.translation_ += collisionMapInfo.move;

	// 天井接触による落下開始(02_07 スライド38枚目)
	if (collisionMapInfo.ceiling) {
		velocity_.y = 0;
	}

	// 02_08 スライド27枚目 壁接触している場合の処理
	UpdateOnWall(collisionMapInfo);

	// 接地判定
	UpdateOnGround(collisionMapInfo);
	/*
	    //02_08 スライド22枚目まで実装したら
	    //（↑でUpdateOnGround関数実装したら）コメントアウト

	    //移動
	    bool landing = false;

	    // 下降あり？
	    if (velocity_.y < 0) {
	        // Y座標が地面以下になったら着地
	        if (worldTransform_.translation_.y <= 1.0f) {
	            landing = true;
	        }
	    }

	    // 接地判定
	    if (onGround_) {
	        // ジャンプ開始
	        if (velocity_.y > 0.0f) {
	            onGround_ = false;
	        }
	    }else {
	        // 着地
	        if (landing) {
	            worldTransform_.translation_.y = 1.0f;
	            velocity_.x *= (1.0f - kAttenuation);
	            velocity_.y  = 0.0f;
	            onGround_    = true;
	        }
	    }
	*/
	// 旋回制御
	if (turnTimer_ > 0.0f) {
		// タイマーを進める
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);

		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	 // ★ 攻撃タイマー進行（最後にまとめてでOK）
	const float dt = 1.0f / 60.0f;
	if (attackCooldown_ > 0.0f) {
		attackCooldown_ = std::max(0.0f, attackCooldown_ - dt);
	}
	if (isAttacking_) {
		attackTimer_ += dt;
		if (attackTimer_ >= kAttackDuration) {
			isAttacking_ = false;
			attackCooldown_ = kAttackCooldown;
		}
	}

	// ワールド行列更新（アフィン変換～DirectXに転送）
	WorldTransformUpdate(worldTransform_);

	// --- ここから追加：無敵＆点滅処理 ---
	if (invincibleTimer_ > 0.0f) {
		invincibleTimer_ = std::max(invincibleTimer_ - (1.0f / 60.0f), 0.0f);

		// 点滅。半透明ON/OFFでもOKだが、分かりやすく非表示/表示のトグル
		// 無敵中は時間に応じてブリンク
		float phase = invincibleTimer_ * kBlinkHz * 2.0f; // 1秒で2*kBlinkHz回トグル
		bool visible = (static_cast<int>(phase) % 2) == 0;

		Vector4 col = {1, 1, 1, visible ? 1.0f : 0.0f};
		playerColor_.SetColor(col);
	} else {
		// 通常時は不透明
		playerColor_.SetColor({1, 1, 1, 1});
	}
	// --- ここまで追加 ---

	// === 攻撃入力と斬撃表示 ===
	// Zキーor任意キー（必要なら別キーに変更）
	if (Input::GetInstance()->TriggerKey(DIK_Z)) {
		// ここで「内部の攻撃ロジック（当たり判定の開始）」も既に行っている想定
		// 見た目だけ追加する

		if (attackModel_) {
			// プレイヤーの向きからオフセットを計算
			// 右向き（+X）/左向き（-X）の左右に少し前方＆上へ
			float yaw = worldTransform_.rotation_.y;
			float dir = (lrDirection_ == LRDirection::kRight) ? +1.0f : -1.0f;

			Vector3 spawnPos = GetWorldPosition();
			spawnPos.x += dir * 0.7f; // 前方
			spawnPos.y += 0.6f;       // 少し上

			// 見た目：ロールで斬撃に傾きを付ける（好みで調整）
			float roll = (dir > 0.0f) ? +0.35f : -0.35f;

			auto* s = new AttackSlash();
			s->Initialize(
			    attackModel_, camera_, spawnPos, yaw,
			    /*life*/ 0.15f,
			    /*scaleStart*/ {0.8f, 0.8f, 0.8f},
			    /*scaleEnd*/ {1.6f, 0.6f, 1.0f}, roll);
			slashes_.push_back(s);
		}
	}

	// スラッシュ更新＆終了したものを破棄
	for (auto it = slashes_.begin(); it != slashes_.end();) {
		AttackSlash* s = *it;
		s->Update();
		if (s->Finished()) {
			delete s;
			it = slashes_.erase(it);
		} else {
			++it;
		}
	}
}

void Player::Draw() {

	// モデル描画
	// model_->Draw(worldTransform_, *camera_);
	// 変更：色オブジェクトを渡す（点滅反映）
	model_->Draw(worldTransform_, *camera_, &playerColor_);
	// 既存：モデル描画
	model_->Draw(worldTransform_, *camera_);

	// 追加：斬撃描画
	for (AttackSlash* s : slashes_) {
		s->Draw();
	}
}

// 02_10 10枚目
Vector3 Player::GetWorldPosition() const {

	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}



// 02_10 14枚目
AABB Player::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::TryStartAttack() {
	if (isAttacking_)
		return;
	if (attackCooldown_ > 0.0f)
		return;
	if (!Input::GetInstance()->TriggerKey(DIK_Z))
		return; // ★ Zキーで攻撃

	isAttacking_ = true;
	attackTimer_ = 0.0f;
	// 立ちモーションや微停止を入れたいなら velocity_.x *= 0.6f; など
}

AABB Player::GetAttackAABB() const {
	// 攻撃が出ていないときは「空」のAABBを返す（呼び側でIsAttackActiveを見るのが本筋）
	AABB aabb{};
	if (!isAttacking_)
		return aabb;

	Vector3 wp = GetWorldPosition();
	// 攻撃中心をプレイヤーの前方にオフセット
	float dir = (lrDirection_ == LRDirection::kRight) ? +1.0f : -1.0f;
	Vector3 center = wp + Vector3(dir * (kWidth / 2.0f + kAttackReach / 2.0f), 0.0f, 0.0f);

	aabb.min = {center.x - kAttackWidth / 2.0f, center.y - kAttackHeight / 2.0f, center.z - kAttackWidth / 2.0f};
	aabb.max = {center.x + kAttackWidth / 2.0f, center.y + kAttackHeight / 2.0f, center.z + kAttackWidth / 2.0f};
	return aabb;
}

// 02_10 21枚目
void Player::OnCollision(const Enemy* enemy) {

	// 不使用
	(void)enemy;

	// 02_12 12枚目 書き換え
	// isDead_ = true;
	TakeDamage(1);
	// ★ 変更：即死させずに無敵化（重複発動を防止）
	if (!IsInvincible()) {
		StartInvincible(kInvincibleDuration);

		// もしノックバックを入れたい場合
		velocity_.x = (lrDirection_ == LRDirection::kRight ? -0.2f : 0.2f);
		velocity_.y = 0.25f;
	}
}
void Player::CleanupSlashes() {
	for (auto* s : slashes_)
		delete s;
	slashes_.clear();
}
