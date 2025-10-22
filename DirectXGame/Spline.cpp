#include "Spline.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

Vector3 Spline::Catmull(int i, float u) const {
	const Vector3& p0 = cp_[i - 1];
	const Vector3& p1 = cp_[i];
	const Vector3& p2 = cp_[i + 1];
	const Vector3& p3 = cp_[i + 2];
	float u2 = u * u, u3 = u2 * u;
	Vector3 r;
	r.x = 0.5f * ((2 * p1.x) + (-p0.x + p2.x) * u + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * u2 + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * u3);
	r.y = 0.5f * ((2 * p1.y) + (-p0.y + p2.y) * u + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * u2 + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * u3);
	r.z = 0.5f * ((2 * p1.z) + (-p0.z + p2.z) * u + (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * u2 + (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * u3);
	return r;
}
Vector3 Spline::CatmullTangent(int i, float u) const {
	const Vector3& p0 = cp_[i - 1];
	const Vector3& p1 = cp_[i];
	const Vector3& p2 = cp_[i + 1];
	const Vector3& p3 = cp_[i + 2];
	float u2 = u * u;
	Vector3 r;
	r.x = 0.5f * ((-p0.x + p2.x) + 2 * (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * u + 3 * (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * u2);
	r.y = 0.5f * ((-p0.y + p2.y) + 2 * (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * u + 3 * (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * u2);
	r.z = 0.5f * ((-p0.z + p2.z) + 2 * (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * u + 3 * (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * u2);
	return r;
}

Vector3 Spline::Pos(float t) const {
	int segs = SegmentCount();
	if (segs <= 0) {
		return {0, 0, 0};
	}
	t = std::clamp(t, 0.0f, (float)segs - 1e-4f);
	int i = (int)std::floor(t) + 1; // p(i-1..i+2) を使用
	float u = t - std::floor(t);
	return Catmull(i, u);
}
Vector3 Spline::Tan(float t) const {
	int segs = SegmentCount();
	if (segs <= 0) {
		return {0, 0, 1};
	}
	t = std::clamp(t, 0.0f, (float)segs - 1e-4f);
	int i = (int)std::floor(t) + 1;
	float u = t - std::floor(t);
	return Normalized(CatmullTangent(i, u));
}
