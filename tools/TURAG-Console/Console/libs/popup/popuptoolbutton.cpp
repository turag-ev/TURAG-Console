#include "popuptoolbutton.h"
#include "popupwidget.h"

#include <QAction>
#include <QMoveEvent>
#include <QPoint>
#include <QResizeEvent>


PopupToolButton::PopupToolButton(PopupContainerWidget* container_, QWidget *parent) :
	QToolButton(parent), popup(nullptr), container(container_)
{
	if (!container) return;

	connect(this, &QToolButton::triggered, this, &PopupToolButton::handlePopupAction);
	connect(container, &QObject::destroyed, this, &PopupToolButton::clearObjectPointer);
	connect(container, &PopupContainerWidget::popupOpened, this, &PopupToolButton::handleOpenedPopup);
	connect(container, &PopupContainerWidget::popupClosed, this, &PopupToolButton::handleClosedPopup);
}

void PopupToolButton::setPopup(PopupWidget *popup_, PopupContainerWidget::DisplayDirection direction_, PopupContainerWidget::DisplayType type_) {
	if (!container || !popup_) return;

	if (popup) {
		disconnect(popup, &QObject::destroyed, this, &PopupToolButton::clearObjectPointer);
	}

	popup = popup_;
	direction = direction_;
	connect(popup, &QObject::destroyed, this, &PopupToolButton::clearObjectPointer);
	container->registerPopup(popup, getPopupPosition(), direction, type_);
}


void PopupToolButton::resizeEvent(QResizeEvent *) {
	if (!popup || !container) return;

	container->updatePopupPosition(popup, getPopupPosition());
}

void PopupToolButton::moveEvent(QMoveEvent *) {
	if (!popup || !container) return;

	container->updatePopupPosition(popup, getPopupPosition());
}

void PopupToolButton::handlePopupAction(QAction *action) {
	if (!action || !container) return;

	if (action->isCheckable()) {
		if (action->isChecked()) {
			container->openPopup(popup);
		} else {
			container->closePopup(popup);
		}
	} else {
		container->togglePopup(popup);
	}
}

void PopupToolButton::clearObjectPointer(QObject *destroyed) {
	if (destroyed == container) {
		container = nullptr;
	} else if (destroyed == popup) {
		popup = nullptr;
	}
}

void PopupToolButton::handleClosedPopup(PopupWidget *popup_) {
	if (popup && popup == popup_) {
		if (defaultAction()) {
			if (defaultAction()->isCheckable()) {
				defaultAction()->setChecked(false);
			}
		} else if (isCheckable()) {
			setChecked(false);
		}
	}
}

void PopupToolButton::handleOpenedPopup(PopupWidget *popup_) {
	if (popup && popup == popup_) {
		if (defaultAction()) {
			if (defaultAction()->isCheckable()) {
				defaultAction()->setChecked(true);
			}
		} else if (isCheckable()) {
			setChecked(true);
		}
	}
}

QPoint PopupToolButton::getPopupPosition() const {
	if (!container) return QPoint();

	// calculate coordinates of button relative to container
	QPoint contPos(container->mapFromGlobal(mapToGlobal(QPoint(0, 0))));

	switch (direction) {
	case PopupContainerWidget::DisplayDirection::toBottom:
		contPos.setY(contPos.y() + height());
		break;

	case PopupContainerWidget::DisplayDirection::toRight:
		contPos.setX(contPos.x() + width());
		break;

	case PopupContainerWidget::DisplayDirection::toLeft:
	case PopupContainerWidget::DisplayDirection::toTop:
		break;
	}

	if (contPos.x() < 0) {
		contPos.setX(0);
	}
	if (contPos.y() < 0) {
		contPos.setY(0);
	}
	if (contPos.x() > container->width()) {
		contPos.setX(container->width());
	}
	if (contPos.y() > container->height()) {
		contPos.setY(container->height());
	}

	return contPos;
}
