#include "Bullet.h"
#include "MyMath.h"

using namespace KamataEngine;

Bullet::Bullet() {
	wt.Initialize();
	wt.scale_ = {1, 1, 1};
	wt.rotation_ = {0, 0, 0};
	wt.translation_ = {0, 0, 0};
	wt.matWorld_ = MakeAffineMatrix(wt.scale_, wt.rotation_, wt.translation_);
	wt.TransferMatrix();
}

void BulletManager::Initialize() {
	const size_t N = 256; // 必要に応じて拡張
	pool_.clear();
	pool_.resize(N); // Bullet() が呼ばれる
}

void BulletManager::SetModels(Model* normal, Model* charged) {
	modelNormal_ = normal;
	modelCharged_ = charged;
}

Bullet* BulletManager::Alloc() {
	for (auto& b : pool_) {
		if (!b.alive) {
			b.alive = true;
			return &b;
		}
	}
	return nullptr; // プール満杯なら破棄（必要なら拡張）
}

void BulletManager::SpawnNormal(const Vector3& pos, const Vector3& dir) {
	if (Bullet* b = Alloc()) {
		b->pos = pos;
		b->vel = dir * speedNormal_;
		b->life = lifeNormal_;
		b->radius = radiusNormal_;
		b->piercing = false;
		b->wt.scale_ = {0.15f, 0.15f, 0.15f};
	}
}

void BulletManager::SpawnCharged(const Vector3& pos, const Vector3& dir) {
	if (Bullet* b = Alloc()) {
		b->pos = pos;
		b->vel = dir * speedCharged_;
		b->life = lifeCharged_;
		b->radius = radiusCharged_;
		b->piercing = true;
		b->wt.scale_ = {0.30f, 0.30f, 0.30f};
	}
}

void BulletManager::Update(float dt) {
	for (auto& b : pool_) {
		if (!b.alive)
			continue;

		b.life -= dt;
		if (b.life <= 0.0f) {
			b.alive = false;
			continue;
		}

		b.pos += b.vel * dt;

		// 見た目更新
		b.wt.translation_ = b.pos;
		b.wt.rotation_ = {0, 0, 0}; // 向きを合わせたければここを調整
		b.wt.matWorld_ = MakeAffineMatrix(b.wt.scale_, b.wt.rotation_, b.wt.translation_);
		b.wt.TransferMatrix();
	}
}

void BulletManager::Draw(Camera& cam) {
	for (auto& b : pool_) {
		if (!b.alive)
			continue;
		if (b.piercing) {
			if (modelCharged_)
				modelCharged_->Draw(b.wt, cam);
		} else {
			if (modelNormal_)
				modelNormal_->Draw(b.wt, cam);
		}
	}
}
