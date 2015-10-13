/***************************************************************************
 * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#ifndef _LUXCOREAPP_H
#define	_LUXCOREAPP_H

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "luxcore/luxcore.h"
#include "logwindow.h"
#include "statswindow.h"
#include "samplerwindow.h"

#define LA_ARRAYSIZE(_ARR)	((int)(sizeof(_ARR) / sizeof(*_ARR)))

class LuxCoreApp {
public:
	LuxCoreApp(luxcore::RenderConfig *renderConfig);
	~LuxCoreApp();

	void RunApp();

	static void LogHandler(const char *msg);
	static void ColoredLabelText(const ImVec4 &col, const char *label, const char *fmt, ...) IM_PRINTFARGS(3);
	static void ColoredLabelText(const char *label, const char *fmt, ...) IM_PRINTFARGS(2);
	
	static ImVec4 colLabel;

	// Mouse "grab" mode. This is the natural way cameras are usually manipulated
	// The flag is off by default but can be turned on by using the -m switch
	bool optMouseGrabMode;

	friend class SamplerWindow;
	friend class StatsWindow;

private:
	static void GLFW_KeyCallBack(GLFWwindow *window, int key, int scanCode, int action, int mods);
	static void GLFW_MouseButtonCallBack(GLFWwindow *window, int button, int action, int mods);
	static void GLFW_MousePositionCallBack(GLFWwindow *window, double x, double y);

	void UpdateMoveStep();
	void SetRenderingEngineType(const std::string &engineType);
	void EditRenderConfig(const luxrays::Properties &samplerProps);
	void SetFilmResolution(const u_int filmWidth, const u_int filmHeight);
	void IncScreenRefreshInterval();
	void DecScreenRefreshInterval();

	void RefreshRenderingTexture();
	void DrawRendering();
	void DrawTiles(const luxrays::Property &propCoords,
		const luxrays::Property &propPasses, const luxrays::Property &propErrors,
		const u_int tileCount, const u_int tileWidth, const u_int tileHeight);
	void DrawTiles();
	void DrawCaptions();

	void MenuRendering();
	void MenuEngine();
	void MenuSampler();
	void MenuFilm();
	void MenuWindow();
	void MenuScreen();
	void MainMenuBar();

	static LogWindow *currentLogWindow;

	SamplerWindow samplerWindow;
	LogWindow logWindow;
	StatsWindow statsWindow;

	luxcore::RenderSession *session;
	luxcore::RenderConfig *config;

	GLuint renderFrameBufferTexID;
	GLFWwindow *window;

	// ImGui inputs
	int newFilmSize[2];
	
	bool optRealTimeMode;
	float optMoveScale;
	float optMoveStep;
	float optRotateStep;

	bool mouseButton0, mouseButton2;
	double mouseGrabLastX, mouseGrabLastY;
	double lastMouseUpdate;
};

#define LA_LOG(a) { std::stringstream _LUXCOREUI_LOG_LOCAL_SS; _LUXCOREUI_LOG_LOCAL_SS << a; LuxCoreApp::LogHandler(_LUXCOREUI_LOG_LOCAL_SS.str().c_str()); }

#endif	/* _LUXCOREAPP_H */
