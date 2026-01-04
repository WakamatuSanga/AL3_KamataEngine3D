#pragma once
#include "IScene.h"
#include "KamataEngine.h"

class ClearScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~ClearScene() override;
};