#pragma once
#include "KamataEngine.h"
#include "PlayerMovement.h"
#include "RailCamera.h"
using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model) {
		model_ = model;
		world_.Initialize();
		world_.scale_ = {0.7f, 0.7f, 0.7f};
		move_.Initialize();
	}
	void Update(const RailCamera& rc) { lastRollZ_ = move_.Update(world_, rc); }
	void Draw(Camera& cam) {
		if (model_)
			model_->Draw(world_, cam);
	}

	void SetViewPlaneDist(float d) { move_.SetViewPlaneDist(d); }
	void SetScreenBiasY(float frac) { move_.SetScreenBiasY(frac); }

private:
	WorldTransform world_{};
	Model* model_ = nullptr;
	PlayerMovement move_;
	float lastRollZ_ = 0.0f;
};
