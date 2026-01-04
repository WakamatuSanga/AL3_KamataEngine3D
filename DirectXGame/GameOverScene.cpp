#include "GameOverScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

using namespace KamataEngine;

void GameOverScene::Initialize() {}

void GameOverScene::Update() {
	// スペースキーでタイトルへ戻る
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(new TitleScene());
	}
}

void GameOverScene::Draw() {
}

GameOverScene::~GameOverScene() {}