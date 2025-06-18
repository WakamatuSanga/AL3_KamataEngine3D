
#include "KamataEngine.h"

class Enemy {
	// 敵キャラ

	enum LRDirection {
		kRiget,
		kLeft,
	};

	// 角
	enum Corner {
		kRightBottom,   // 右下
		kLeftBottom,    // 左下
		kRightTop,	    // 右上
		kLeftTop,	    // 左上
					    
		kNumCorner	    // 要素数
	};
private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	//std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformEnemy_;
	KamataEngine::Vector3 velocity_ = {};

	// 移動
	static inline const float kAcceleration = 0.2f;

	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 0.1f;

	// 自機の回転
	LRDirection lrDirection_ = LRDirection::kLeft;
	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.3f;
	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.1f;
	public:
	// 初期化
	    void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, KamataEngine::Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();




};
