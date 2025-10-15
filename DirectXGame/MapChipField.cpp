#include "MapChipField.h"
#include <cassert>
#include <fstream>
#include <sstream>

const float MapChipField::kBlockWidth = 2.0f;
const float MapChipField::kBlockHeight = 2.0f;

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		OutputDebugStringA(("CSV open failed: " + filePath + "\n").c_str());
		// 既定は全ブランク
		mapChipData_.assign(kNumBlockVertical, std::vector<MapChipType>(kNumBlockHorizontal, MapChipType::kBlank));
		return;
	}

	mapChipData_.assign(kNumBlockVertical, std::vector<MapChipType>(kNumBlockHorizontal, MapChipType::kBlank));

	std::string line;
	for (uint32_t y = 0; y < kNumBlockVertical && std::getline(file, line); ++y) {
		// 先頭BOM除去
		if (y == 0 && line.size() >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
			line.erase(0, 3);
		}
		std::istringstream ls(line);
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			std::string word;
			if (!std::getline(ls, word, ','))
				break;
			if (!word.empty() && word.back() == '\r')
				word.pop_back(); // CR除去
			// 空白除去
			while (!word.empty() && isspace((unsigned char)word.back()))
				word.pop_back();
			while (!word.empty() && isspace((unsigned char)word.front()))
				word.erase(word.begin());

			mapChipData_[y][x] = (word == "1") ? MapChipType::kBlock : MapChipType::kBlank;
		}
	}

	// 読めたブロック数をログ
	int count = 0;
	for (auto& row : mapChipData_)
		for (auto t : row)
			if (t == MapChipType::kBlock)
				++count;
	OutputDebugStringA((std::string("Loaded blocks: ") + std::to_string(count) + "\n").c_str());
}


MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t x, uint32_t y) {
	// 範囲外なら空白を返す
	if (x >= kNumBlockHorizontal || y >= kNumBlockVertical) {
		return MapChipType::kBlank;
	}
	return mapChipData_[y][x];
}

KamataEngine::Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t x, uint32_t y) {
	return {
	    kBlockWidth * x,
	    kBlockHeight * (kNumBlockVertical - 1 - y), // Y座標は下から数える
	    0.0f};
}