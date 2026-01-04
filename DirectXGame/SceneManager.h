#pragma once
#include "IScene.h"

class SceneManager {
public:
	// シングルトンインスタンスの取得
	static SceneManager* GetInstance();

	// 次のシーンを予約する（引数には new されたシーンを渡す）
	void ChangeScene(IScene* newScene);

	// 更新処理（シーン切り替え処理もここで行う）
	void Update();

	// 描画処理
	void Draw();

	// 終了処理（メモリ解放）
	void Finalize();

private:
	// コンストラクタを隠蔽（シングルトン）
	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	IScene* currentScene_ = nullptr; // 現在実行中のシーン
	IScene* nextScene_ = nullptr;    // 次に切り替わる予定のシーン
};