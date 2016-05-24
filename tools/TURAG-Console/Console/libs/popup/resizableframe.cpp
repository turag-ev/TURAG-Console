#include "resizableframe.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>

#include <algorithm>


ResizableFrame::ResizableFrame(QWidget *mainWidget_, bool movable_, QWidget *parent) :
	QFrame(parent), mainWidget(mainWidget_), movable(movable_), borderSize(defaultBorderSize)
{
	if (!mainWidget) {
		mainWidget = new QWidget;
	}
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(mainWidget);
	setLayout(layout);
	setBorderGripSize(std::max(layout->spacing() - 2, 1));

	resizeBorders = ResizeBorder::all;
	setMouseTracking(true);
}

ResizableFrame::~ResizableFrame() {
}

void ResizableFrame::setResizeBorders(ResizableFrame::ResizeBorder resizeBorders_)
{
	resizeBorders = resizeBorders_;

	if (resizeBorders == ResizeBorder::none) {
		setMouseTracking(false);
	} else {
		setMouseTracking(true);
	}
}

bool ResizableFrame::isResizeBorderSet(ResizableFrame::ResizeBorder borders)
{
	int setBorders = static_cast<int>(resizeBorders);
	int compareBorders = static_cast<int>(borders);

	return ((setBorders & compareBorders) == compareBorders);
}

void ResizableFrame::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		if (event->x() > borderSize &&
				event->x() < width() - borderSize &&
				event->y() > borderSize &&
				event->y() < height() - borderSize)
		{
			if (movable) {
				startPos = StartPositions::move;
				setCursor(Qt::SizeAllCursor);
			} else {
				startPos = StartPositions::none;
				setCursor(Qt::ArrowCursor);
			}
		}

		dragStartPosition = event->pos();
		dragStartGeometry = geometry();
	}
}

void ResizableFrame::mouseMoveEvent(QMouseEvent *event) {
	if (!(event->buttons() & Qt::LeftButton)) {
	// No drag, just change the cursor and return

		if (event->x() <= borderSize && event->y() <= borderSize &&
				isResizeBorderSet(ResizeBorder::left_top)) {
			startPos = StartPositions::topleft;
			setCursor(Qt::SizeFDiagCursor);
		} else if (event->x() <= borderSize && event->y() >= height() - borderSize &&
				   isResizeBorderSet(ResizeBorder::left_bottom)) {
			startPos = StartPositions::bottomleft;
			setCursor(Qt::SizeBDiagCursor);
		} else if (event->x() >= width() - borderSize && event->y() <= borderSize &&
				   isResizeBorderSet(ResizeBorder::top_right)) {
			startPos = StartPositions::topright;
			setCursor(Qt::SizeBDiagCursor);
		} else if (event->x() >= width() - borderSize && event->y() >= height() - borderSize &&
				   isResizeBorderSet(ResizeBorder::right_bottom)) {
			startPos = StartPositions::bottomright;
			setCursor(Qt::SizeFDiagCursor);
		} else if (event->x() <= borderSize &&
				   isResizeBorderSet(ResizeBorder::left)) {
			startPos = StartPositions::left;
			setCursor(Qt::SizeHorCursor);
		} else if (event->x() >= width() - borderSize &&
				   isResizeBorderSet(ResizeBorder::right)) {
			startPos = StartPositions::right;
			setCursor(Qt::SizeHorCursor);
		} else if (event->y() <= borderSize &&
				   isResizeBorderSet(ResizeBorder::top)) {
			startPos = StartPositions::top;
			setCursor(Qt::SizeVerCursor);
		} else if (event->y() >= height() - borderSize &&
				   isResizeBorderSet(ResizeBorder::bottom)) {
			startPos = StartPositions::bottom;
			setCursor(Qt::SizeVerCursor);
		} else {
			if (movable) {
				startPos = StartPositions::move;
				setCursor(Qt::SizeAllCursor);
			} else {
				startPos = StartPositions::none;
				setCursor(Qt::ArrowCursor);
			}
		}
		return;
	}

	switch (startPos) {
	case StartPositions::topleft:
		setGeometry(dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
					dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
					dragStartGeometry.width() + (dragStartPosition.x() - event->x()),
					height() + (dragStartPosition.y() - event->y()));
		dragStartGeometry = geometry();
		emit geometryChanged();
		break;

	case StartPositions::bottomleft:
		setGeometry(dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
					dragStartGeometry.top(),
					dragStartGeometry.width() + (dragStartPosition.x() - event->x()),
					event->y());
		dragStartGeometry = geometry();
		emit geometryChanged();
		break;

	case StartPositions::topright:
		setGeometry(dragStartGeometry.left(),
					dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
					event->x(),
					height() + (dragStartPosition.y() - event->y()));
		dragStartGeometry = geometry();
		emit geometryChanged();
		break;

	case StartPositions::bottomright:
		setGeometry(dragStartGeometry.left(),
					dragStartGeometry.top(),
					event->x(),
					event->y());
		emit geometryChanged();
		break;

	case StartPositions::left:
		setGeometry(dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
					dragStartGeometry.top(),
					dragStartGeometry.width() + (dragStartPosition.x() - event->x()),
					height());
		dragStartGeometry = geometry();
		emit geometryChanged();
		break;

	case StartPositions::right:
		setGeometry(dragStartGeometry.left(),
					dragStartGeometry.top(),
					event->x(),
					height());
		emit geometryChanged();
		break;

	case StartPositions::top:
		setGeometry(dragStartGeometry.left(),
					dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
					dragStartGeometry.width(),
					height() + (dragStartPosition.y() - event->y()));
		dragStartGeometry = geometry();
		emit geometryChanged();
		break;

	case StartPositions::bottom:
		setGeometry(dragStartGeometry.left(),
					dragStartGeometry.top(),
					width(),
					event->y());
		emit geometryChanged();
		break;

	case StartPositions::move:
		setGeometry(dragStartGeometry.left() - (dragStartPosition.x() - event->x()),
					dragStartGeometry.top() - (dragStartPosition.y() - event->y()),
					width(),
					height());
		dragStartGeometry = geometry();
		emit geometryChanged();
		break;

	case StartPositions::none:
		break;
	}
}

