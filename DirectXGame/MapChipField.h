#pragma once
#include "KamataEngine.h"
#include <vector>

enum class MapChipType {
	kBlank,
	kBlock,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	// 範囲矩形
	struct Rect {
		float left;
		float rigt;
		float bottom;
		float top;
	};

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; };

	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; };

	MapChipData mapChipData_;

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
};