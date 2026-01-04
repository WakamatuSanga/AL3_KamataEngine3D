#include "TitleScene.h"
#include "GameScene.h"
#include "SceneManager.h"
#include <imgui.h>

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
	// 文字表示（本来はSpriteなどが望ましいですが、ImGuiで代用）
	ImGui::Begin("TITLE SCREEN", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowFontScale(2.0f);
	ImGui::Text("RAIL SHOOTER GAME");
	ImGui::Separator();
	ImGui::Text("Press SPACE to Start");
	ImGui::End();
}

TitleScene::~TitleScene() {
	// 解放処理
}