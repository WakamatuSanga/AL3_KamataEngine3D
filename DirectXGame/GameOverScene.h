#pragma once
#include "IScene.h"
#include "KamataEngine.h"

class GameOverScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~GameOverScene() override;
};