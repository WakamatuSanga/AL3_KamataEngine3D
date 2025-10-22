#pragma once
#include "Fade.h"
#include "KamataEngine.h"

using namespace KamataEngine;

// 02_12 19枚目 タイトルシーン
class TitleScene {
public:
    // 02_12 27枚目 シーンのフェーズ
    enum class Phase {
        kFadeIn,  // フェードイン
        kMain,    // メイン部
        kFadeOut, // フェードアウト
    };

    ~TitleScene();

    void Initialize();

    void Update();

    void Draw();

    // 0212 26枚目
    bool IsFinished() const { return finished; }

private:
    static inline const float kTimeTitleMove = 2.0f;

    // ビュープロジェクション
    Camera camera;
    WorldTransform worldTransformTitle;
    WorldTransform worldTransformPlayer;

    Model* modelPlayer = nullptr;
    Model* modelTitle = nullptr;

    float counter = 0.0f;
    // 0212 26枚目
    bool finished = false;

    // 0213 12枚目
    Fade* fade = nullptr;

    // 0213 27枚目 現在のフェーズ
    Phase phase = Phase::kFadeIn;
};
