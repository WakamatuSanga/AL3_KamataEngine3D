#include "Ground.h"
#include "MyMath.h"

using namespace KamataEngine;

void Ground::Initialize(Model* model, float width, float segLen, int countZ, float y, float speed, int columns, float colSpacingX) {
	model_ = model ? model : Model::Create();
	width_ = width;
	segLen_ = segLen;
	countZ_ = max(1, countZ);
	y_ = y;
	scrollSpeed_ = speed;
	objMode_ = false;
	columns_ = max(1, columns);
	colSpacingX_ = colSpacingX;

	tiles_.clear();
	tiles_.reserve(countZ_ * columns_);

	// Z行 × X列 の順で配置（行優先でフラット化）
	for (int zi = 0; zi < countZ_; ++zi) {
		for (int ci = 0; ci < columns_; ++ci) {
			auto wt = std::make_unique<WorldTransform>();
			wt->Initialize();

			// 板方式：薄い箱
			wt->scale_ = {width_, 0.5f, segLen_};
			wt->rotation_ = {0, 0, 0};
			wt->translation_ = {ColumnOffsetX_(ci), y_, zi * segLen_};

			WorldTransformUpdate(*wt);
			tiles_.push_back(std::move(wt));
		}
	}
}

void Ground::InitializeOBJ(Model* model, float stepZ, int countZ, float y, float speed, float uniformScale, int columns, float colSpacingX) {
	model_ = model ? model : Model::Create();
	segLen_ = stepZ;
	countZ_ = max(1, countZ);
	y_ = y;
	scrollSpeed_ = speed;
	objMode_ = true;
	objUniformScale_ = uniformScale;
	columns_ = max(1, columns);
	colSpacingX_ = colSpacingX;

	tiles_.clear();
	tiles_.reserve(countZ_ * columns_);

	for (int zi = 0; zi < countZ_; ++zi) {
		for (int ci = 0; ci < columns_; ++ci) {
			auto wt = std::make_unique<WorldTransform>();
			wt->Initialize();

			// OBJそのまま（等倍スケールのみ）
			wt->scale_ = {objUniformScale_, objUniformScale_, objUniformScale_};
			wt->rotation_ = {0, 0, 0};
			wt->translation_ = {ColumnOffsetX_(ci), y_, zi * segLen_};

			WorldTransformUpdate(*wt);
			tiles_.push_back(std::move(wt));
		}
	}
}

void Ground::SetY(float y) {
	y_ = y;
	for (int i = 0; i < (int)tiles_.size(); ++i) {
		auto& wt = tiles_[i];
		wt->translation_.y = y_;
		WorldTransformUpdate(*wt);
	}
}

void Ground::Update() {
	// スクロール
	for (int i = 0; i < (int)tiles_.size(); ++i) {
		auto& wt = tiles_[i];

		// 行/列をインデックスから復元（行優先）
		const int ci = i % columns_;
		// const int zi = i / columns_; // 必要なら使用

		wt->translation_.z -= scrollSpeed_;
		// 列の中心 + 視差
		wt->translation_.x = ColumnOffsetX_(ci) - playerX_ * parallaxX_;
		wt->translation_.y = y_;
		wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
		wt->TransferMatrix();
	}

	// 再配置のしきい値
	float recycleZ = -segLen_ - recycleBackExtra_;
	if (recycleBehindCamera_) {
		recycleZ = cameraZ_ - recycleBehindMore_;
	}

	// 手前に来たタイルを奥へ
	const float addZ = segLen_ * countZ_;
	for (auto& wt : tiles_) {
		if (wt->translation_.z < recycleZ) {
			wt->translation_.z += addZ;
			wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
			wt->TransferMatrix();
		}
	}
}

void Ground::StartAtCameraFront(float cameraZ, int tilesInFront, float margin) {
	if (tiles_.empty())
		return;
	tilesInFront = std::clamp(tilesInFront, 0, countZ_);

	const float firstZ = cameraZ - margin - tilesInFront * segLen_;

	// 行優先のため、Z行ごとにZをセットし、列はColumnOffsetX_で配置
	for (int zi = 0; zi < countZ_; ++zi) {
		const float z = firstZ + zi * segLen_;
		for (int ci = 0; ci < columns_; ++ci) {
			const int idx = zi * columns_ + ci;
			auto& wt = tiles_[idx];
			wt->translation_.z = z;
			wt->translation_.x = ColumnOffsetX_(ci) - playerX_ * parallaxX_;
			wt->translation_.y = y_;
			wt->matWorld_ = MakeAffineMatrix(wt->scale_, wt->rotation_, wt->translation_);
			wt->TransferMatrix();
		}
	}
}

void Ground::StartAtRecycleLine(float cameraZ) {
	if (tiles_.empty())
		return;

	float recycleZ = -segLen_ - recycleBackExtra_;
	if (recycleBehindCamera_) {
		recycleZ = cameraZ - recycleBehindMore_;
	}
	const float firstZ = recycleZ + 1e-3f; // ほんの少し手前から

	for (int zi = 0; zi < countZ_; ++zi) {
		const float z = firstZ + zi * segLen_;
		for (int ci = 0; ci < columns_; ++ci) {
			const int idx = zi * columns_ + ci;
			auto& wt = tiles_[idx];
			wt->translation_.z = z;
			wt->translation_.x = ColumnOffsetX_(ci) - playerX_ * parallaxX_;
			wt->translation_.y = y_;
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
