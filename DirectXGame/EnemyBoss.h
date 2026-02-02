#pragma once
#include "EnemyBullet.h"
#include "EnemyHomingBullet.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector>

class EnemyBoss {
public:
	~EnemyBoss();

	// 初期化
	void Initialize(KamataEngine::Model* bossModel, KamataEngine::Model* bulletModel, KamataEngine::Model* homingBulletModel, Player* player);

	// 更新
	void Update(const KamataEngine::Vector3& cameraPos);

	// 3D描画
	void Draw(KamataEngine::Camera& camera);

	// 2D UI（HPバー＆警告）描画
	void DrawUI();

	// 衝突時の処理
	void OnCollision();

	// ステータス取得
	bool IsDead() const { return isDead_; }
	int GetHP() const { return hp_; }
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	float GetCollisionRadius() const;

	// 弾リスト取得
	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }
	const std::vector<EnemyHomingBullet*>& GetHomingBullets() const { return homingBullets_; }

	// 出現位置セット
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }

private:
	// 行動フェーズ
	enum class Phase {
		Approach,        // 登場
		AttackAime,      // 精密狙撃
		AttackHoming,    // 連続ホーミング
		AttackSpread,    // 高密度拡散弾
		AttackSpiral,    // 螺旋弾
		AttackDanmaku,   // 花火弾幕
		AttackTouhou,    // 東方風全方位ホーミング
		AttackBeamRight, // 右側ビーム
		AttackBeamLeft,  // 左側ビーム
		Standby          // 待機
	};

	// 攻撃関数
	void FireAimedBullet();
	void FireHomingBullet();
	void FireSpreadBullet();
	void FireSpiralBullet();
	void FireDanmaku();
	void FireTouhouHoming();
	void FireBeam(bool isRight);

	// 移動先の決定
	void DecideNextPosition(const KamataEngine::Vector3& cameraPos);

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	Player* player_ = nullptr;

	KamataEngine::Model* bulletModel_ = nullptr;
	KamataEngine::Model* homingBulletModel_ = nullptr;

	std::vector<EnemyBullet*> bullets_;
	std::vector<EnemyHomingBullet*> homingBullets_;

	int hp_ = 1000;
	float maxHp_ = 1000.0f;
	bool isDead_ = false;

	Phase phase_ = Phase::Approach;
	int phaseTimer_ = 0;

	// 移動制御用
	KamataEngine::Vector3 targetPos_;
	int moveTimer_ = 0;

	const float kModelScale_ = 6.0f;

	// HPバー用スプライト
	KamataEngine::Sprite* hpBarBG_ = nullptr;
	KamataEngine::Sprite* hpBar_ = nullptr;

	// ★追加：警告エリア表示用スプライト
	KamataEngine::Sprite* warningArea_ = nullptr;

	// SE
	uint32_t seShoot_ = 0;
	uint32_t seHit_ = 0;
	uint32_t seDead_ = 0;
};