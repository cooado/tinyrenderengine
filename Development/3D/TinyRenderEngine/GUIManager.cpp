#include "GUIManager.h"

namespace TRE
{
	GUIManager::GUIMessage& GUIManager::AddGUIMessage()
	{
		GUIManager::GUIMessage message;
		m_GUIMessages.push_back(message);
		return m_GUIMessages.back();
	};

	void GUIManager::LoadFromFile(std::string path)
	{
	};

	Window* GUIManager::GetWindow(std::string name)
	{
		return m_Windows[name];
	};

	void GUIManager::SetDesktopSize(INT32 width, INT32 height)
	{
		m_DesktopWidth = width;
		m_DesktopHeight = height;
	};

	void GUIManager::Update()
	{
		for(std::vector<GUIMessage>::const_iterator i = m_GUIMessages.begin(); i != m_GUIMessages.end(); i++)
		{
			if((*i).Message == GUI_MouseMove)
			{
			}
			else if((*i).Message == GUI_MouseLeftDown)
			{
			}
			else if((*i).Message == GUI_MouseLeftUp)
			{
			}
			else if((*i).Message == GUI_MouseRightDown)
			{
			}
			else if((*i).Message == GUI_MouseRightUp)
			{
			}
			else if((*i).Message == GUI_KeyDown)
			{
			}
			else if((*i).Message == GUI_KeyUp)
			{
			}
		};
		m_GUIMessages.clear();
	};

	void GUIManager::Draw()
	{
		2DRenderer::GetInstance()->Render();
	};

};