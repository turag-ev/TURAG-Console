#include "popupcontainerwidget.h"
#include "popupwidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPalette>
#include <QPoint>
#include <QSize>

#include <algorithm>


PopupContainerWidget::PopupContainerWidget(QWidget *mainWidget_, QWidget *parent) :
	QWidget(parent), mainWidget(mainWidget_)
{
	if (!mainWidget) {
		mainWidget = new QWidget;
	}

	mainWidget->setParent(this);
	mainWidget->resize(size());
	mainWidget->show();

	modalOverlay = new QWidget;
	modalOverlay->setParent(this);
	modalOverlay->installEventFilter(this);
	modalOverlay->hide();

	specialOverlay = new QWidget;
	specialOverlay->setParent(this);
	specialOverlay->installEventFilter(this);
	specialOverlay->setAutoFillBackground(true);
	QPalette overlay_palette = specialOverlay->palette();
	overlay_palette.setColor(QPalette::Window, QColor(0, 0, 0, 220));
	specialOverlay->setPalette(overlay_palette);
	specialOverlay->hide();

}

void PopupContainerWidget::registerPopup(
		PopupWidget* popup,
		const QPoint& pos,
		DisplayDirection direction,
		DisplayType type)
{
	popupWidgets.insert(popup, PopupInfo(popup, pos, direction, type));
	popup->setParent(this);
	popup->hide();
}


void PopupContainerWidget::resizeEvent(QResizeEvent *) {
	mainWidget->resize(size());

	if (modalOverlay->isVisible()) {
		modalOverlay->resize(size());
	}
	if (specialOverlay->isVisible()) {
		specialOverlay->resize(size());
	}

	for (const PopupInfo& popupInfo : popupWidgets.values()) {
		if (popupInfo.popup->isVisible()) {
			adjustPopupGeometry(popupInfo.popup);
		}
	}
}

bool PopupContainerWidget::eventFilter(QObject *watched, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			if (watched == modalOverlay) {
				// close all opened popups which are modal
				// (this should always be only one)
				for (const PopupInfo& popupInfo : popupWidgets.values()) {
					if (popupInfo.popup->isVisible() && popupInfo.type == DisplayType::modal) {
						closePopup(popupInfo);
					}
				}
				return true;
			} else if (watched == specialOverlay) {
				// close all opened popups which are special
				// (this should always be only one)
				for (const PopupInfo& popupInfo : popupWidgets.values()) {
					if (popupInfo.popup->isVisible() && popupInfo.type == DisplayType::special) {
						closePopup(popupInfo);
					}
				}
				return true;
			}
		}
	}
	return false;
}

void PopupContainerWidget::openPopup(PopupWidget* popup) {
	if (popup == nullptr) return;
	PopupInfo popupInfo(popupWidgets.value(popup));
	if (popupInfo.popup == nullptr) return;

	openPopup(popupInfo);
}

void PopupContainerWidget::closePopup(PopupWidget* popup) {
	if (popup == nullptr) return;
	PopupInfo popupInfo(popupWidgets.value(popup));
	if (popupInfo.popup == nullptr) return;

	closePopup(popupInfo);
}

void PopupContainerWidget::togglePopup(PopupWidget *popup) {
	if (popup == nullptr) return;
	PopupInfo popupInfo(popupWidgets.value(popup));
	if (popupInfo.popup == nullptr) return;

	togglePopup(popupInfo);
}


void PopupContainerWidget::openPopup(const PopupInfo &popupinfo) {
	if (popupinfo.popup->isVisible()) return;

	// opened modal popups are always closed if a different
	// popup is to be opened (this should never concern more than
	// one  popup)
	for (const PopupInfo& popupInfo : popupWidgets.values()) {
		if (popupInfo.popup->isVisible() && popupInfo.type == DisplayType::modal) {
			closePopup(popupInfo);
		}
	}

	// close opened special popups if we open a different special one
	// (this should never concern more than one)
	if (popupinfo.type == DisplayType::special) {
		for (const PopupInfo& popupInfo : popupWidgets.values()) {
			if (popupInfo.popup->isVisible() && popupInfo.type == DisplayType::special) {
				closePopup(popupInfo);
			}
		}

		specialOverlay->resize(size());
		specialOverlay->raise();
		specialOverlay->show();
	}

	if (popupinfo.type == DisplayType::modal) {
		modalOverlay->resize(size());
		modalOverlay->raise();
		modalOverlay->show();
	}


	// all types of popups are resized, raised and shown
	adjustPopupGeometry(popupinfo);
	popupinfo.popup->raise();
	popupinfo.popup->show();
	emit popupOpened(popupinfo.popup);
}

void PopupContainerWidget::closePopup(const PopupInfo &popupinfo) {
	if (!popupinfo.popup->isVisible()) return;

	if (popupinfo.type == DisplayType::modal) {
		modalOverlay->hide();
	}
	if (popupinfo.type == DisplayType::special) {
		specialOverlay->hide();
	}

	popupinfo.popup->hide();
	emit popupClosed(popupinfo.popup);
}

void PopupContainerWidget::togglePopup(const PopupContainerWidget::PopupInfo &popupinfo) {
	if (popupinfo.popup->isVisible()) {
		closePopup(popupinfo);
	} else {
		openPopup(popupinfo);
	}
}



void PopupContainerWidget::updatePopupPosition(PopupWidget *popup, const QPoint &pos) {
	if (popup == nullptr) return;
	PopupInfo popupInfo(popupWidgets.value(popup));
	if (popupInfo.popup == nullptr) return;

	popupInfo.position = pos;
	popupWidgets.insert(popup, popupInfo);

	adjustPopupGeometry(popupInfo);
}

void PopupContainerWidget::adjustPopupGeometry(PopupWidget* popup) const {
	if (popup == nullptr) return;
	PopupInfo popupInfo(popupWidgets.value(popup));
	if (popupInfo.popup == nullptr) return;

	adjustPopupGeometry(popupInfo);
}

void PopupContainerWidget::adjustPopupGeometry(const PopupContainerWidget::PopupInfo &popupInfo) const {
	QPoint newPosition;
	QSize newSize;
	PopupWidget* popup = popupInfo.popup;

	switch (popupInfo.direction) {
	case DisplayDirection::toBottom:
		// vertical
		if (popup->getPreferredSize().height() + popupInfo.position.y() <= size().height()) {
			newSize.setHeight(popup->getPreferredSize().height());
		} else {
			newSize.setHeight(std::max(popup->minimumHeight(), size().height() - popupInfo.position.y()));
		}
		newPosition.setY(popupInfo.position.y());

		// horizontal
		if (popup->getPreferredSize().width() + popupInfo.position.x() <= size().width()) {
			newSize.setWidth(popup->getPreferredSize().width());
			newPosition.setX(popupInfo.position.x());
		} else if (popup->getPreferredSize().width() <= size().width()) {
			newSize.setWidth(popup->getPreferredSize().width());
			newPosition.setX(size().width() - popup->getPreferredSize().width());
		} else {
			newSize.setWidth(std::max(popup->minimumWidth(), size().width()));
			newPosition.setX(0);
		}
		break;

	case DisplayDirection::toRight:
		// horizontal
		if (popup->getPreferredSize().width() + popupInfo.position.x() <= size().width()) {
			newSize.setWidth(popup->getPreferredSize().width());
		} else {
			newSize.setWidth(std::max(popup->minimumWidth(), size().width() - popupInfo.position.x()));
		}
		newPosition.setX(popupInfo.position.x());

		// vertical
		if (popup->getPreferredSize().height() + popupInfo.position.y() <= size().height()) {
			newSize.setHeight(popup->getPreferredSize().height());
			newPosition.setY(popupInfo.position.y());
		} else if (popup->getPreferredSize().height() <= size().height()) {
			newSize.setHeight(popup->getPreferredSize().height());
			newPosition.setY(size().height() - popup->getPreferredSize().height());
		} else {
			newSize.setHeight(std::max(popup->minimumHeight(), size().height()));
			newPosition.setY(0);
		}
		break;

	case DisplayDirection::toTop:
		// vertical
		if (popup->getPreferredSize().height() <= popupInfo.position.y()) {
			newSize.setHeight(popup->getPreferredSize().height());
			newPosition.setY(popupInfo.position.y() - popup->getPreferredSize().height());
		} else {
			int newHeight = std::max(popup->minimumHeight(), popupInfo.position.y());
			newSize.setHeight(newHeight);
			newPosition.setY(popupInfo.position.y() - newHeight);
		}

		// horizontal
		if (popup->getPreferredSize().width() + popupInfo.position.x() <= size().width()) {
			newSize.setWidth(popup->getPreferredSize().width());
			newPosition.setX(popupInfo.position.x());
		} else if (popup->getPreferredSize().width() <= size().width()) {
			newSize.setWidth(popup->getPreferredSize().width());
			newPosition.setX(size().width() - popup->getPreferredSize().width());
		} else {
			newSize.setWidth(std::max(popup->minimumWidth(), size().width()));
			newPosition.setX(0);
		}
		break;

	case DisplayDirection::toLeft:
		// horizontal
		if (popup->getPreferredSize().width() <= popupInfo.position.x()) {
			newSize.setWidth(popup->getPreferredSize().width());
			newPosition.setY(popupInfo.position.x() - popup->getPreferredSize().width());
		} else {
			int newWidth = std::max(popup->minimumWidth(), popupInfo.position.x());
			newSize.setWidth(newWidth);
			newPosition.setX(popupInfo.position.x() - newWidth);
		}

		// vertical
		if (popup->getPreferredSize().height() + popupInfo.position.y() <= size().height()) {
			newSize.setHeight(popup->getPreferredSize().height());
			newPosition.setY(popupInfo.position.y());
		} else if (popup->getPreferredSize().height() <= size().height()) {
			newSize.setHeight(popup->getPreferredSize().height());
			newPosition.setY(size().height() - popup->getPreferredSize().height());
		} else {
			newSize.setHeight(std::max(popup->minimumHeight(), size().height()));
			newPosition.setY(0);
		}
		break;
	}

	if (newPosition != popup->pos()) {
		popup->move(newPosition);
	}
	if (newSize != popup->size()) {
		popup->resize(newSize);
	}
}

