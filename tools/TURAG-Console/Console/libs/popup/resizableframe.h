#ifndef RESIZABLEFRAME_H
#define RESIZABLEFRAME_H

#include <QFrame>

class QMouseEvent;


class ResizableFrame : public QFrame {
	Q_OBJECT

public:
	constexpr static int defaultBorderSize = 5;

	enum class StartPositions {topleft, left, bottomleft, bottom, bottomright, right, topright, top, move, none};

	explicit ResizableFrame(QWidget *mainWidget_, bool movable = false, QWidget *parent = 0);
	~ResizableFrame();

	void setBorderGripSize(int size) {borderSize = size; }

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
	int borderSize;
};

#endif // RESIZABLEFRAME_H
