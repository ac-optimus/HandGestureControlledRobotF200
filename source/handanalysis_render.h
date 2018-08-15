
/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include <list>
#include <map>
#include "pxcsensemanager.h"
#include <pxchandmodule.h>//this is necessary to include the handmodule for hand tracking
#include "util_render.h"
#include <vector>
#include <pxchanddata.h>

class HandRender : public UtilRender {//Handrender is subclass of UtilRender.
public:
	HandRender(pxcCHAR *title = 0) :UtilRender(title) {};
	bool RenderFrame(PXCImage *rgbImage, PXCHandModule *detector,
		PXCHandData::JointData nodes[][PXCHandData::NUMBER_OF_JOINTS],
		pxcCHAR gestureName[][PXCHandData::MAX_NAME_SIZE], PXCHandData::BodySideType *handSide);

protected:

	virtual void DrawMore(HDC hdc, double scale_x, double scale_y);

	struct Line {
		int x0, y0;
		int x1, y1;
	};

	struct Node {
		int x, y;
		int radius;
		COLORREF color;
	};

	std::list<Line>     m_lines;
	std::map<std::pair<int, PXCHandData::JointType>, Node> m_nodes;  //int is for hand id

	struct Gesture {
		PXCHandData::BodySideType handSide;
		pxcCHAR name[PXCHandData::MAX_NAME_SIZE * 2];
	};

	std::list<Gesture>  m_gestures;
};

#pragma once
