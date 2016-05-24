#ifndef RESIZABLEFRAME_H
#define RESIZABLEFRAME_H

#include <QFrame>

class QMouseEvent;


class ResizableFrame : public QFrame {
	Q_OBJECT

public:
	constexpr static int defaultBorderSize = 5;

	enum class StartPositions {topleft, left, bottomleft, bottom, bottomright, right, topright, top, move, none};
	enum class ResizeBorder {left = 0x01, top = 0x02, right = 0x04, bottom = 0x08, left_top = 0x03, left_right = 0x05,
							 left_bottom = 0x09, top_right = 0x06, top_bottom = 0x0a, right_bottom = 0x0c,
							 left_top_right = 0x07, top_right_bottom = 0x0e, right_bottom_left = 0x0d,
							 bottom_left_top = 0x0b, all = 0x0f, none = 0x00};

	explicit ResizableFrame(QWidget *mainWidget_, bool movable = false, QWidget *parent = 0);
	~ResizableFrame();

	void setBorderGripSize(int size) {borderSize = size; }
	void setResizeBorders(ResizeBorder resizeBorders);
	ResizeBorder getResizeBorders(void) const { return resizeBorders; }
	bool isResizeBorderSet(ResizeBorder borders);

signals:
	void geometryChanged(void);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);

private:

	QPoint dragStartPosition;
	QRect dragStartGeometry;
	StartPositions startPos;
	QWidget* mainWidget;
	bool movable;
	ResizeBorder resizeBorders;
	int borderSize;
};

#endif // RESIZABLEFRAME_H
