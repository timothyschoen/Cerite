#pragma once

#include "Edge.h"

class Canvas;
class LooseEdge : public Edge
{
public:

	LooseEdge(Canvas* parent, ValueTree node);

	~LooseEdge()
	{
		//canvas->repaint();
	}

	void addToCanvas() override;
	void removeFromCanvas() override;


private:
	void mouseUp(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseDown(const MouseEvent& e) override;

	int shouldSnap() override;

	String getTooltip() override
	{
		return String();
	};
};
