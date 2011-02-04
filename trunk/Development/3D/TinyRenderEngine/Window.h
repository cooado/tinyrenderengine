#ifndef _TRE_WINDOW_H__
#define _TRE_WINDOW_H__

#include <list>

namespace TRE
{

enum DrawType
{
	DT_AlphaBlend,
	DT_AlphaMask
};

class Window
{
public:
	std::list<Window*> m_ChildWindows;
	Window* m_ParentWindow;
	// Relative to its parent
	Rect m_Rect;
	// Relative to desktop
	Rect m_GlobalRect;
	DrawType m_DrawType;

	void Draw(Rect* rect, float depth);
	void Show(bool show);
	void BringToTop();

};

};

#endif