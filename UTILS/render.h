#pragma once
#include "../SDK/ISurface.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9core.h"


#define D3D_COLOR_BLACK(a)	D3DCOLOR_ARGB(a, 0, 0, 0)
#define D3D_COLOR_GREEN(a)	D3DCOLOR_ARGB(a, 0, 255, 0)

namespace SDK
{
	class IMaterial;
}
namespace RENDER
{
	unsigned int CreateF(std::string font_name, int size, int weight, int blur, int scanlines, int flags);
	void PolygonOutline(int count, SDK::Vertex_t* Vertexs, CColor color, CColor colorLine);
	void Polygon(int count, SDK::Vertex_t* Vertexs, CColor color);
	void PolyLine(int *x, int *y, int count, CColor color);
	void Clear(int x, int y, int w, int h, CColor color);
	void DrawF(int X, int Y, unsigned int Font, bool center_width, bool center_height, CColor Color, std::string Input);
	void DrawWF(int X, int Y, unsigned int Font, CColor Color, const wchar_t* Input);
	Vector2D GetTextSize(unsigned int Font, std::string Input);
	void Text(int x, int y, CColor color, DWORD font, const char* text);
	void Textf(int x, int y, CColor color, DWORD font, const char* fmt, ...);
	void DrawLine(int x1, int y1, int x2, int y2, CColor color);

	void DrawEmptyRect(int x1, int y1, int x2, int y2, CColor color, unsigned char = 0); // the flags are for which sides to ignore in clockwise, 0b1 is top, 0b10 is right, etc.
	void DrawFilledRect(int x1, int y1, int x2, int y2, CColor color);
	void FillRectangle(int x1, int y2, int width, int height, CColor color);
	void DrawFilledRectOutline(int x1, int y1, int x2, int y2, CColor color);
	void DrawFilledRectArray(SDK::IntRect* rects, int rect_amount, CColor color);
	void DrawCornerRect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const bool outlined, const CColor& color, const CColor& outlined_color);
	void DrawEdges(float topX, float topY, float bottomX, float bottomY, float length, CColor color);

	void DrawCircle(int x, int y, int radius, int segments, CColor color);
	void DrawFilledCircle(int x, int y, int radius, int segments, CColor color);
	//void DrawFilledCircle(Vector2D center, CColor color, CColor outline, float radius, float points);
	void FilledCircle(Vector2D position, float points, float radius, CColor color);
	void TexturedPolygon(int n, std::vector<SDK::Vertex_t> vertice, CColor color);
	void DrawSomething();
	void DrawFilled3DBox(Vector origin, int width, int height, CColor outline, CColor filling);
	bool WorldToScreen(Vector world, Vector &screen);
}