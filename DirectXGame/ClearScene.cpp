#include "ClearScene.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <imgui.h>

using namespace KamataEngine;

void ClearScene::Initialize() {}

void ClearScene::Update() {
	// スペースキーでタイトルへ戻る
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(new TitleScene());
	}
}

void ClearScene::Draw() {
	ImGui::Begin("GAME CLEAR", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowFontScale(2.0f);
	ImGui::Text("MISSION ACCOMPLISHED!");
	ImGui::Separator();
	ImGui::Text("Press SPACE to Return to Title");
	ImGui::End();
}

ClearScene::~ClearScene() {}