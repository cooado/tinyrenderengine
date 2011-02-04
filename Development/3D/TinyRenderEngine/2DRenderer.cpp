#include "2DRenderer.h"

namespace TRE
{
	2DRenderer::DrawCommand& 2DRenderer::AddDrawCommand(DrawType dt)
	{
		DrawCommand dc;
		switch(dt)
		{
		case DT_AlphaBlend:
			m_DrawCommandAlphaBlend.push_back(dc);
			return m_DrawCommandAlphaBlend.back();
			break;
		case DT_AlphaMask:
			m_DrawCommandAlphaMask.push_back(dc);
			return m_DrawCommandAlphaMask.back();
			break;
		};
	};

	void 2DRenderer::Render()
	{
	};

};