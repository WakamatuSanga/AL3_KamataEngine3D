#pragma once
#include "IScene.h"
#include "KamataEngine.h"

class TitleScene : public IScene {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	~TitleScene() override;

private:
	// タイトルで表示したいモデルなどがあればここに追加
	// 今はシンプルにImGuiなどで文字を表示します
};