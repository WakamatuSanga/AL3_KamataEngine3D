#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "MyMath.h"
#include "skydome.h"

using namespace KamataEngine;

class GameOverScene {
public:
	enum class Phase { kFadeIn, kMain, kFadeOut };

	~GameOverScene() {
		delete modelFont_;
		delete fade_;
	}

	void Initialize() {
		camera_.Initialize();

		// ▼資産名は手持ちに合わせて（無ければ "titleFont" 等）
		//modelFont_ = Model::CreateFromOBJ("gameOverFont", true);
		 modelFont_ = Model::CreateFromOBJ("titleFont", true); // ←代替

		wtFont_.Initialize();
		wtFont_.scale_ = {8.0f, 8.0f, 8.0f};
		wtFont_.translation_.y = 2.0f;
		WorldTransformUpdate(wtFont_);
		modelBg_ = Model::CreateFromOBJ("SkyDome", true);
		bgDome_ = new Skydome();
		bgDome_->Initialize(modelBg_, &camera_);
		bgDome_->SetColor({0, 0, 0, 1}); // 黒
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
		if (bgDome_)
			bgDome_->Draw();
		modelFont_->Draw(wtFont_, camera_);
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
