// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace sses;
using namespace hg::Utils;
using namespace ssvu;

namespace hg
{
	void HexagonGame::draw()
	{
		styleData.computeColors();

		window.clear(Color::Black);

		if(!Config::getNoBackground()) { backgroundCamera.apply(); styleData.drawBackground(window, ssvs::zeroVec2f, getSides()); }

		if(Config::get3D())
		{
			status.drawing3D = true;

			float effect{styleData._3dSkew * Config::get3DMultiplier() * status.pulse3D};
			Vec2f skew{1.f, 1.f + effect};
			backgroundCamera.setSkew(skew);

			for(auto i(0u); i < depthCameras.size(); ++i)
			{
				Camera& depthCamera(depthCameras[i]);
				depthCamera.setView(backgroundCamera.getView());
				depthCamera.setSkew(skew);
				depthCamera.setOffset({0, styleData._3dSpacing * (float(i + 1.f) * styleData._3dPerspectiveMult) * (effect * 3.6f)});

				status.overrideColor = getColorDarkened(styleData.get3DOverrideColor(), styleData._3dDarkenMult);
				status.overrideColor.a /= styleData._3dAlphaMult;
				status.overrideColor.a -= i * styleData._3dAlphaFalloff;

				depthCamera.apply();
				wallQuads.clear(); playerTris.clear(); manager.draw();
				render(wallQuads); render(playerTris);
			}

			status.drawing3D = false;
		}

		backgroundCamera.apply();
		wallQuads.clear(); playerTris.clear(); manager.draw();
		render(wallQuads); render(playerTris);

		overlayCamera.apply();
		drawText();

		if(Config::getFlash()) render(flashPolygon);
		if(mustTakeScreenshot) { window.saveScreenshot("screenshot.png"); mustTakeScreenshot = false; }
	}

	void HexagonGame::initFlashEffect()
	{
		flashPolygon.clear();
		flashPolygon.emplace_back(Vec2f{-100.f, -100.f}, Color{255, 255, 255, 0});
		flashPolygon.emplace_back(Vec2f{Config::getWidth() + 100.f, -100.f}, Color{255, 255, 255, 0});
		flashPolygon.emplace_back(Vec2f{Config::getWidth() + 100.f, Config::getHeight() + 100.f}, Color{255, 255, 255, 0});
		flashPolygon.emplace_back(Vec2f{-100.f, Config::getHeight() + 100.f}, Color{255, 255, 255, 0});
	}

	void HexagonGame::drawText()
	{
		ostringstream s;
		s << "time: " << toStr(status.currentTime).substr(0, 5) << "\n";

		if(levelStatus.tutorialMode) s << "tutorial mode" << "\n";
		else if(Config::getOfficial()) s << "official mode" << "\n";

		if(Config::getDebug()) s << "debug mode" << "\n";
		if(levelStatus.swapEnabled) s << "swap enabled" << "\n";
		if(Config::getInvincible()) s << "invincibility on" << "\n";
		if(status.scoreInvalid) s << "score invalidated (performance issues)" << "\n";
		if(status.hasDied) s << "press r to restart" << "\n";
		if(Config::getShowFPS()) s << "FPS: " << window.getFPS() << "\n";

		const auto& trackedVariables(levelStatus.trackedVariables);
		if(Config::getShowTrackedVariables() && !trackedVariables.empty())
		{
			s << "\n";
			for(const auto& t : trackedVariables)
			{
				if(!lua.doesVariableExist(t.variableName)) continue;
				string var{lua.readVariable<string>(t.variableName)};
				s << t.displayName << ": " << var << "\n";
			}
		}

		s.flush();

		const Vec2f pos{15, 3};
		const vector<Vec2f> offsets{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

		Color offsetColor{getColor(1)};
		if(Config::getBlackAndWhite()) offsetColor = Color::Black;
		text.setString(s.str());
		text.setCharacterSize(25 / Config::getZoomFactor());
		text.setOrigin(0, 0);

		text.setColor(offsetColor);
		for(const auto& o : offsets) { text.setPosition(pos + o); render(text); }

		text.setColor(getColorMain());
		text.setPosition(pos);
		render(text);

		if(messageText.getString() == "") return;

		messageText.setOrigin(getGlobalWidth(messageText) / 2.f, 0);
		messageText.setColor(offsetColor);
		for(const auto& o : offsets) { messageText.setPosition(Vec2f{Config::getWidth() / 2.f, Config::getHeight() / 6.f} + o); render(messageText); }

		messageText.setColor(getColorMain());
		render(messageText);
	}
}
