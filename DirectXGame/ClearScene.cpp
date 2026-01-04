#include "ClearScene.h"
#include "SceneManager.h"
#include "TitleScene.h"

using namespace KamataEngine;

void ClearScene::Initialize() {}

void ClearScene::Update() {
	// スペースキーでタイトルへ戻る
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(new TitleScene());
	}
}

void ClearScene::Draw() {
}

ClearScene::~ClearScene() {}