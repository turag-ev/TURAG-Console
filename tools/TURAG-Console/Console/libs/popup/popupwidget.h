#ifndef TURAG_CONSOLE_CONSOLE_LIBS_POPUP_POPUPWIDGET_H
#define TURAG_CONSOLE_CONSOLE_LIBS_POPUP_POPUPWIDGET_H

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

    static constexpr int minPreferredWidth = 100;
    static constexpr int minPreferredHeight = 100;


	explicit PopupWidget(
			QWidget* mainWidget,
			QSize defaultPreferredSize = QSize(),
			QString saveGeometryIdentifier = QString());

	QSize getPreferredSize(void) const { return preferredSize; }

protected:
	virtual void hideEvent(QHideEvent *event);

private:
    void correctPreferredSize(void);

	QSize preferredSize;

	QString saveGeometryIdentifier;
};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_POPUP_POPUPWIDGET_H
