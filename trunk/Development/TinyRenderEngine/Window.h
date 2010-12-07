#ifndef _TRE_WINDOW_H__
#define _TRE_WINDOW_H__

#include <list>

class Window
{
public:
	std::list<Window*> m_ChildWindows;
	Window* m_ParentWindow;

	void Draw(Rect* rect, float depth);
};

#endif