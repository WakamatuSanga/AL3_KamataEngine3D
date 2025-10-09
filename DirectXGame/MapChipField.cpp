#include "MapChipField.h"
#include <cassert>
#include <fstream>
#include <sstream>

const float MapChipField::kBlockWidth = 2.0f;
const float MapChipField::kBlockHeight = 2.0f;

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// ファイルの内容を文字列ストリームにコピー
	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	// mapChipData_をリサイズ
	mapChipData_.resize(kNumBlockVertical);
	for (auto& line : mapChipData_) {
		line.resize(kNumBlockHorizontal);
	}

	// 1行ずつ読み込んでマップチップデータを生成
	std::string line;
	for (uint32_t y = 0; y < kNumBlockVertical; ++y) {
		getline(mapChipCsv, line);
		std::istringstream lineStream(line);
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			std::string word;
			getline(lineStream, word, ',');

			if (word == "1") {
				mapChipData_[y][x] = MapChipType::kBlock;
			} else {
				mapChipData_[y][x] = MapChipType::kBlank;
			}
		}
	}
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