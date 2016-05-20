#ifndef POPUPWIDGET_H
#define POPUPWIDGET_H

#include "resizableframe.h"

#include <QSize>
#include <QString>

class QHideEvent;
class QResizeEvent;
class QWidget;


class PopupWidget : public ResizableFrame
{
public:
	static constexpr int defaultPreferredWidth = 300;
	static constexpr int defaultPreferredHeight = 300;


	explicit PopupWidget(
			QWidget* mainWidget,
			QSize defaultPreferredSize = QSize(),
			QString saveGeometryIdentifier = QString());

	QSize getPreferredSize(void) const { return preferredSize; }

protected:
	virtual void hideEvent(QHideEvent *event);

private:
	QSize preferredSize;

	QString saveGeometryIdentifier;
};

#endif // POPUPWIDGET_H
