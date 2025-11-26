#include "Ground.h"
#include "MyMath.h"
#include <algorithm>

using namespace KamataEngine;

void Ground::Initialize(Model* model, float width, float segLen, int count, float y, float speed) {
	model_ = model;
	width_ = width;
	segLen_ = segLen;
	count_ = max(1, count);
	y_ = y;
	scrollSpeed_ = speed;
	objMode_ = false; // 板方式

	tiles_.clear();
	tiles_.reserve(count_);

	// Z+ 方向に連結配置
	for (int i = 0; i < count_; ++i) {
		auto wt = std::make_unique<WorldTransform>();
		wt->Initialize();

		// ★ 板方式：薄い箱を横長に伸ばす（Yを薄く）
		wt->scale_ = {width_, 0.5f, segLen_};
		wt->rotation_ = {0.0f, 0.0f, 0.0f};
		wt->translation_ = {0.0f, y_, i * segLen_};

		// ワールド行列を反映
		WorldTransformUpdate(*wt);
		tiles_.push_back(std::move(wt));
	}
}
void Ground::StartAtRecycleLine(float cameraZ) {
	if (tiles_.empty() || count_ <= 0)
		return;

	// 今の Update() と同じロジックで「リサイクル閾値」を再現
	float recycleZ = -segLen_ - recycleBackExtra_; // 既定
	if (recycleBehindCamera_) {
		recycleZ = cameraZ - recycleBehindMore_; // カメラ基準
	}

	// 閾値すぐ手前（わずかに前）から並べる
	const float firstZ = recycleZ + 1e-3f;

	for (int i = 0; i < count_; ++i) {
		auto& wt = tiles_[i];
		wt->translation_.z = firstZ + i * segLen_;
		wt->translation_.x = -playerX_ * parallaxX_;
		wt->translation_.y = y_;
		wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
		wt->TransferMatrix();
	}
}
void Ground::InitializeOBJ(Model* model, float stepZ, int count, float y, float speed, float uniformScale) {
	model_ = model ? model : Model::Create();
	segLen_ = stepZ; // タイル間の奥行間隔
	count_ = max(1, count);
	y_ = y;
	scrollSpeed_ = speed;
	objMode_ = true; // OBJ形状そのまま
	objUniformScale_ = uniformScale;

	tiles_.clear();
	tiles_.reserve(count_);

	for (int i = 0; i < count_; ++i) {
		auto wt = std::make_unique<WorldTransform>();
		wt->Initialize();

		// ★ OBJ方式：形状を潰さずに等倍スケールだけ適用
		wt->scale_ = {objUniformScale_, objUniformScale_, objUniformScale_};
		wt->rotation_ = {0.0f, 0.0f, 0.0f};
		wt->translation_ = {0.0f, y_, i * segLen_};

		WorldTransformUpdate(*wt);
		tiles_.push_back(std::move(wt));
	}
}

void Ground::SetY(float y) {
	y_ = y;
	for (auto& wt : tiles_) {
		wt->translation_.y = y_;
		WorldTransformUpdate(*wt);
	}
}

void Ground::Update() {
	// スクロール＆（任意）視差
	for (auto& wt : tiles_) {
		wt->translation_.z -= scrollSpeed_;
		wt->translation_.x = -playerX_ * parallaxX_;
		wt->translation_.y = y_; // 高さは固定
		// 傾きはつけない（OBJの凹凸をそのまま見せるため回転0）
		// スケールもモードに応じて維持（Initialize/InitializeOBJで設定済み）

		wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
		wt->TransferMatrix();
	}
	// 再配置のしきい値を決定
	float recycleZ = -segLen_ - recycleBackExtra_; // 既定よりさらに奥
	if (recycleBehindCamera_) {
		recycleZ = cameraZ_ - recycleBehindMore_; // カメラより more だけ後ろ
	}

	// 手前に来たタイルを奥へ再配置（ループ）
	const float addZ = segLen_ * count_;
	for (auto& wt : tiles_) {
		if (wt->translation_.z < recycleZ) {
			wt->translation_.z += addZ;
			wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
			wt->TransferMatrix();
		}
	}
}

void Ground::Draw(Camera& cam) {
	if (!model_)
		return;
	for (auto& wt : tiles_) {
		model_->Draw(*wt, cam);
	}
}
