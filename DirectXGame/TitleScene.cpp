#include "TitleScene.h"
#include "GameScene.h"
#include "SceneManager.h"

using namespace KamataEngine;

void TitleScene::Initialize() {
	// 必要なリソースのロードなど
}

void TitleScene::Update() {
	// スペースキーでゲーム開始
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// GameSceneへ切り替え
		// ここで new GameScene() することで、ゲームシーンの変数は全て初期化されます
		SceneManager::GetInstance()->ChangeScene(new GameScene());
	}
}

void TitleScene::Draw() {
}

TitleScene::~TitleScene() {
	// 解放処理
}