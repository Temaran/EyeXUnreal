#pragma once
#include "CoreUObject.h"

struct EyeXRect
{
public:
	int Left;
	int Top;
	int Right;
	int Bottom;

	EyeXRect(int left, int top, int right, int bottom)
	{
		Left = left;
		Top = top;
		Right = right;
		Bottom = bottom;
	}

	static bool Intersects(const EyeXRect& A, const EyeXRect& B)
	{
		//  Segments A and B do not intersect when:
		//
		//       (left)   A     (right)
		//         o-------------o
		//  o---o        OR         o---o
		//    B                       B
		//
		//
		// We assume the A and B are well-formed rectangles.
		// i.e. (Top,Left) is above and to the left of (Bottom,Right)
		const bool bDoNotOverlap =
			B.Right < A.Left || A.Right < B.Left ||
			B.Bottom < A.Top || A.Bottom < B.Top;

		return !bDoNotOverlap;
	}
};