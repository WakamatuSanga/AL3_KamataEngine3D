#include "Ground.h"
#include "MyMath.h"

using namespace KamataEngine;

void Ground::Initialize(Model* model) {
	model_ = model;

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// ★サイズ調整
	// 箱（海）なので、プレイヤーを包み込むくらい大きくします
	// ステージの広さに合わせて数値を調整してください
	worldTransform_.scale_ = {50.0f, 50.0f, 50.0f};

	// ★位置調整
	// カメラの原点に合わせて配置します
	worldTransform_.translation_ = {0.0f, -520.0f, 0.0f};

	// 回転行列の初期化（最初は回転なし＝単位行列）
	matRotationAccumulated_ = MakeIdentityMatrix();

	// 行列の更新
	WorldTransformUpdate(worldTransform_);
}

void Ground::Update(const Matrix4x4& playerMat) {

	// 1. プレイヤーの「右方向ベクトル」を行列から取り出す (1行目: X軸)
	Vector3 playerRight = {playerMat.m[0][0], playerMat.m[0][1], playerMat.m[0][2]};
	playerRight = Normalized(playerRight);

	// 2. 回転速度（プレイヤーが進む速さに合わせて調整）
	//    例: 毎フレーム 0.5度回す
	float groundSpeed = 0.5f * (3.141592f / 180.0f);

	// 3. 「プレイヤーの右」を軸に、「手前」に回転させる行列を作る
	Matrix4x4 matRotDelta = MakeRotateAxisAngle(playerRight, -groundSpeed);

	// 4. 今までの回転に合成（蓄積）
	matRotationAccumulated_ *= matRotDelta;

	// 5. 行列の組み立て（自力作成）
	Matrix4x4 matScale = MakeScaleMatrix(worldTransform_.scale_);
	Matrix4x4 matTrans = MakeTranslateMatrix(worldTransform_.translation_);

	// 拡大 → 回転(蓄積) → 平行移動
	worldTransform_.matWorld_ = matScale * matRotationAccumulated_ * matTrans;

	// GPUへ転送
	worldTransform_.TransferMatrix();
}

void Ground::Draw(Camera& camera) {
	if (model_) {
		model_->Draw(worldTransform_, camera);
	}
}