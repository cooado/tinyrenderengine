#ifndef _TRE_2DRENDERER_H__
#define _TRE_2DRENDERER_H__

#include "Prerequisites.h"
#include "Window.h"
#include <vector>

namespace TRE
{
	class 2DRenderer
	{
		DECLARE_SINGLETON(2DRenderer)	
	public:

		struct DrawCommand
		{
			FLOAT32 x, y, z;
			FLOAT32 u, v;
		};

		DrawCommand& AddDrawCommand(DrawType dt);
		void Render();

	private:
		std::vector<DrawCommand> m_DrawCommandAlphaBlend;
		std::vector<DrawCommand> m_DrawCommandAlphaMask;
	};
};

#endif
