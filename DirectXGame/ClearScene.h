#pragma once
#include "Fade.h"
#include "MyMath.h" 
#include "KamataEngine.h"
#include "skydome.h"

using namespace KamataEngine;

class ClearScene {
public:
	enum class Phase { kFadeIn, kMain, kFadeOut };

	~ClearScene() {
		delete modelFont_;
		delete fade_;
	}

	void Initialize() {
		camera_.Initialize();
		// クリア表示用モデル（無ければ "titleFont" 等に変えてください）
		modelFont_ = Model::CreateFromOBJ("clearFont", true);
		wtFont_.Initialize();
		wtFont_.scale_ = {8.0f, 8.0f, 8.0f};
		wtFont_.translation_.y = 2.0f;
		WorldTransformUpdate(wtFont_);
		modelBg_ = Model::CreateFromOBJ("SkyDome", true);
		bgDome_ = new Skydome();
		bgDome_->Initialize(modelBg_, &camera_);
		//bgDome_->SetColor({0.3f, 0.3f, 0.3f, 1}); // 灰

		fade_ = new Fade();
		fade_->Initialize();
		fade_->Start(Fade::Status::FadeIn, 1.0f);
		phase_ = Phase::kFadeIn;
	}

	void Update() {
		switch (phase_) {
		case Phase::kFadeIn:
			fade_->Update();
			if (fade_->IsFinished())
				phase_ = Phase::kMain;
			break;
		case Phase::kMain:
			if (Input::GetInstance()->PushKey(DIK_SPACE)) {
				fade_->Start(Fade::Status::FadeOut, 1.0f);
				phase_ = Phase::kFadeOut;
			}
			break;
		case Phase::kFadeOut:
			fade_->Update();
			if (fade_->IsFinished())
				finished_ = true;
			break;
		}
		camera_.UpdateMatrix();
		WorldTransformUpdate(wtFont_);
	}

	void Draw() {
		auto* dx = DirectXCommon::GetInstance();
		Model::PreDraw(dx->GetCommandList());
		modelFont_->Draw(wtFont_, camera_);
		Model::PreDraw(dx->GetCommandList());
		if (bgDome_)
			bgDome_->Draw();
		Model::PostDraw();

		fade_->Draw();
	}

	bool IsFinished() const { return finished_; }

private:
	Camera camera_;
	WorldTransform wtFont_;
	Model* modelFont_ = nullptr;

	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	bool finished_ = false;
	Skydome* bgDome_ = nullptr;
	Model* modelBg_ = nullptr;
};
