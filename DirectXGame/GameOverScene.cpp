#include "GameOverScene.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <imgui.h>

using namespace KamataEngine;

void GameOverScene::Initialize() {}

void GameOverScene::Update() {
	// スペースキーでタイトルへ戻る
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(new TitleScene());
	}
}

void GameOverScene::Draw() {
	ImGui::Begin("GAME OVER", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowFontScale(2.0f);
	ImGui::Text("GAME OVER...");
	ImGui::Separator();
	ImGui::Text("Press SPACE to Return to Title");
	ImGui::End();
}

GameOverScene::~GameOverScene() {}