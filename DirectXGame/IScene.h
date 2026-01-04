#pragma once

// 全てのシーンの基底クラス（インターフェース）
class IScene {
public:
	// 仮想デストラクタ
	virtual ~IScene() {};

	// 純粋仮想関数（継承先で必ず実装しなければならない関数）
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
};