#pragma once
#include "KamataEngine.h"
#include <string>
#include <vector>

// マップチップの種類
enum class MapChipType {
	kBlank, // 0: 空白
	kBlock, // 1: ブロック
};

class MapChipField {
public:
	// CSVファイルを読み込んでマップチップデータを生成
	void LoadMapChipCsv(const std::string& filePath);

	// 指定したインデックスのマップチップの種類を取得
	MapChipType GetMapChipTypeByIndex(uint32_t x, uint32_t y);

	// 指定したインデックスのマップチップの中心座標を取得
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t x, uint32_t y);

	// 横と縦のブロック数を取得
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }
	uint32_t GetNumBlockVertical() { return kNumBlockVertical; }

private:
	// 1ブロックのサイズ
	static const float kBlockWidth;
	static const float kBlockHeight;

	// マップの広さ
	static const uint32_t kNumBlockHorizontal = 30; // 横
	static const uint32_t kNumBlockVertical = 15;   // 縦

	// マップチップデータ
	std::vector<std::vector<MapChipType>> mapChipData_;
};