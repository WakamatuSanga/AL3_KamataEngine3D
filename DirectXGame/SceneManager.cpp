#include "SceneManager.h"

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}

void SceneManager::ChangeScene(IScene* newScene) {
	// 次のシーンを予約しておく（即座には切り替えない）
	nextScene_ = newScene;
}

void SceneManager::Update() {
	// 切り替え予約がある場合、ここで実行
	if (nextScene_) {
		// 現在のシーンがあれば削除（デストラクタが呼ばれる）
		if (currentScene_) {
			delete currentScene_;
		}
		// 新しいシーンに入れ替え
		currentScene_ = nextScene_;
		nextScene_ = nullptr;

		// 新しいシーンの初期化
		currentScene_->Initialize();
	}

	// 現在のシーンの更新
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::Finalize() {
	// 残っているシーンを削除
	if (currentScene_) {
		delete currentScene_;
		currentScene_ = nullptr;
	}
	if (nextScene_) {
		delete nextScene_;
		nextScene_ = nullptr;
	}
}