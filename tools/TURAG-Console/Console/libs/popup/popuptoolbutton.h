#ifndef TURAG_CONSOLE_CONSOLE_LIBS_POPUP_POPUPTOOLBUTTON_H
#define TURAG_CONSOLE_CONSOLE_LIBS_POPUP_POPUPTOOLBUTTON_H

#include "popupcontainerwidget.h"

#include <QPoint>
#include <QToolButton>

class QAction;
class QMoveEvent;
class QResizeEvent;


class PopupToolButton : public QToolButton
{
public:
	explicit PopupToolButton(PopupContainerWidget* container, QWidget* parent = nullptr);

	void setPopup(
			PopupWidget* popup,
			PopupContainerWidget::DisplayDirection direction = PopupContainerWidget::DisplayDirection::toBottom,
			PopupContainerWidget::DisplayType type = PopupContainerWidget::DisplayType::normal);

protected:
	virtual void resizeEvent(QResizeEvent *event);
	virtual void moveEvent(QMoveEvent *event);

private slots:
	void handlePopupAction(QAction *action);
	void clearObjectPointer(QObject* destroyed);
	void handleClosedPopup(PopupWidget* popup);
	void handleOpenedPopup(PopupWidget* popup);

private:
	QPoint getPopupPosition(void) const;

	PopupWidget* popup;
	PopupContainerWidget* container;
	PopupContainerWidget::DisplayDirection direction;
};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_POPUP_POPUPTOOLBUTTON_H
