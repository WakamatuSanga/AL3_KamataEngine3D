#include "TitleScene.h"
#include "MyMath.h"
#include <numbers>

TitleScene::~TitleScene() {
	delete modelPlayer;
	delete modelTitle;

	// 02_13 12枚目
	delete fade;
}

void TitleScene::Initialize() {

	modelTitle = Model::CreateFromOBJ("titleFont", true);
	modelPlayer = Model::CreateFromOBJ("player");

	// カメラ初期化
	camera.Initialize();

	const float kPlayerTitle = 2.0f;

	worldTransformTitle.Initialize();

	worldTransformTitle.scale_ = {kPlayerTitle, kPlayerTitle, kPlayerTitle};

	const float kPlayerScale = 10.0f;

	worldTransformPlayer.Initialize();

	worldTransformPlayer.scale_ = {kPlayerScale, kPlayerScale, kPlayerScale};

	worldTransformPlayer.rotation_.y = 0.95f * std::numbers::pi_v<float>;

	worldTransformPlayer.translation_.x = -2.0f;

	worldTransformPlayer.translation_.y = -10.0f;

	// 02_13 12枚目
	fade = new Fade();
	fade->Initialize();

	// 02_13 22枚目
	fade->Start(Fade::Status::FadeIn, 1.0f);
}

void TitleScene::Update() {

	// 02_12 27枚目
	// 02_13 13枚目 27枚目で削除
	//	fade_->Update();

	// 02_13 27枚目
	switch (phase) {
	case Phase::kFadeIn:
		fade->Update();

		if (fade->IsFinished()) {
			phase = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade->Start(Fade::Status::FadeOut, 1.0f);
			phase = Phase::kFadeOut;
		}
		break;
	case Phase::kFadeOut:
		fade->Update();
		if (fade->IsFinished()) {
			finished = true;
		}
		break;
	}

	// 02_13 27枚目で↑のPhase::kMainブロックへ
	//	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
	//	finished_ = true;
	//	}

	counter += 1.0f / 60.0f;
	counter = std::fmod(counter, kTimeTitleMove);

	float angle = counter / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformTitle.translation_.y = std::sin(angle) + 10.0f;

	camera.TransferMatrix();

	// アフィン変換～DirectXに転送(タイトル座標)
	WorldTransformUpdate(worldTransformTitle);

	// アフィン変換～DirectXに転送（プレイヤー座標）
	WorldTransformUpdate(worldTransformPlayer);
}

void TitleScene::Draw() {

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);

	modelTitle->Draw(worldTransformTitle, camera);
	modelPlayer->Draw(worldTransformPlayer, camera);

	Model::PostDraw();

	// 02_13 13枚目
	fade->Draw();
}