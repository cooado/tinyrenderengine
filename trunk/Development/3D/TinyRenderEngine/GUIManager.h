#ifndef _TRE_GUIMANAGER_H__
#define _TRE_GUIMANAGER_H__

#include "Prerequisites.h"
#include "Window.h"
#include "2DRenderer.h"
#include <string>
#include <list>
#include <map>

namespace TRE
{
	class GUIManager
	{
		DECLARE_SINGLETON(GUIManager)

	public:
		enum GUIMessageID
		{
			GUI_MouseMove,
			GUI_MouseLeftDown,
			GUI_MouseLeftUp,
			GUI_MouseRightDown,
			GUI_MouseRightUp,
			GUI_KeyDown,
			GUI_KeyUp
		};

		struct GUIMessage
		{
			UINT32 Message;
			UINT32 Param1;
			UINT32 Param2;
		};

		GUIMessage& AddGUIMessage();
		void LoadFromFile(std::string path);
		Window* GetWindow(std::string name);
		void SetDesktopSize(INT32 width, INT32 height);
		void Update();
		void Draw();

	private:
		Window* m_FocusedWindow;
		INT32 m_DesktopWidth;
		INT32 m_DesktopHeight;
		std::list<Window*> m_TopLevelWindows;
		std::vector<GUIMessage> m_GUIMessages;
		std::map<std::string, Window*> m_Windows;
	};
};

#endif _TRE_GUIMANAGER_H__