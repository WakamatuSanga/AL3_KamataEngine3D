#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>

using namespace KamataEngine; // これ書いておくとkamataEngine::書かなくてよい

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2D_26_ワカマツ_サンガ");

	// DirectXCommonインスタスの取得
	DirectXCommon* dxComon = DirectXCommon::GetInstance();

	// ゲームシーンのインスタンス生成//

	GameScene* gameScene = new GameScene();
	// ゲームシーンの初期化

	gameScene->Initialize();
	// メインループ
	while (true) {

		// エンジンの更新
		if (KamataEngine::Update()) {

			break;
		}
		// ゲームシーンの更新
		gameScene->Update();
		// 確認//////
		//  描画開始
		dxComon->PreDraw();
		// ここに描画処理を記述する

		// ゲームシーンの描画
		gameScene->Draw();

		// 描画終了
		dxComon->PostDraw();
	}

	// ゲームシーンの解放
	// nullptrの代入
	gameScene = nullptr;
	delete gameScene;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}