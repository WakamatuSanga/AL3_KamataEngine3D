#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
#include <vector>
using namespace KamataEngine;

// 制御点は最低4点（前後に番兵を1つずつ置くのが安定）
class Spline {
public:
    void SetControlPoints(const std::vector<Vector3>& cp) { cp_ = cp; }
    // t ∈ [0, SegmentCount()) を想定。内部で区間 index とローカル u に分解
    Vector3 Pos(float t) const;
    Vector3 Tan(float t) const; // 正規化接線
    int SegmentCount() const { return (int)cp_.size() - 3; }
private:
    Vector3 Catmull(int i, float u) const;
    Vector3 CatmullTangent(int i, float u) const;
    std::vector<Vector3> cp_;
};
