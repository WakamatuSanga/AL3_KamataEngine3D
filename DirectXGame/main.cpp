#include "GameScene.h"
#include "KamataEngine.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include <Windows.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// 初期化処理
	KamataEngine::Initialize(L"レールシューティングゲーム");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ★ 最初のシーンはタイトルから
	SceneManager::GetInstance()->ChangeScene(new TitleScene());

	// メインループ
	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// ゲームシーンの更新（シーン管理）
		SceneManager::GetInstance()->Update();

		// 描画開始
		dxCommon->PreDraw();

		// ゲームシーンの描画
		SceneManager::GetInstance()->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// シーンマネージャー終了処理（内部で現在のシーンをdelete）
	SceneManager::GetInstance()->Finalize();

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}