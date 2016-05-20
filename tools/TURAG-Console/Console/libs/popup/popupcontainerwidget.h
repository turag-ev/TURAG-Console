#ifndef POPUPCONTAINERWIDGET_H
#define POPUPCONTAINERWIDGET_H

#include <QHash>
#include <QPoint>
#include <QWidget>

class PopupWidget;
class QResizeEvent;


class PopupContainerWidget : public QWidget
{
	Q_OBJECT
public:
	enum class DisplayType { normal, modal, special };
	enum class DisplayDirection { toTop, toBottom, toRight, toLeft };

	struct PopupInfo {
		PopupWidget* popup;
		QPoint position;
		DisplayDirection direction;
		DisplayType type;

		PopupInfo(void) :
			popup(nullptr) {}

		PopupInfo(PopupWidget* popup_, const QPoint& position_, DisplayDirection direction_, DisplayType type_) :
			popup(popup_), position(position_), direction(direction_), type(type_) {}
	};


	explicit PopupContainerWidget(QWidget* mainWidget, QWidget *parent = 0);

	void registerPopup(
			PopupWidget* popup,
			const QPoint& pos,
			DisplayDirection direction = DisplayDirection::toBottom,
			DisplayType type = DisplayType::normal);


signals:
	void popupOpened(PopupWidget* popup);
	void popupClosed(PopupWidget* popup);


public slots:
	void openPopup(PopupWidget* popup);
	void closePopup(PopupWidget* popup);
	void togglePopup(PopupWidget* popup);
	void updatePopupPosition(PopupWidget* popup, const QPoint& pos);


protected:
	virtual void resizeEvent(QResizeEvent *event);
	virtual bool eventFilter(QObject *watched, QEvent *event);

private:
	void adjustPopupGeometry(PopupWidget* popup) const;
	void adjustPopupGeometry(const PopupInfo& popupInfo) const;
	void openPopup(const PopupInfo& popupinfo);
	void closePopup(const PopupInfo& popupinfo);
	void togglePopup(const PopupInfo& popupinfo);

	QWidget* mainWidget;
	QWidget* modalOverlay;
	QWidget* specialOverlay;
	QHash<PopupWidget*, PopupInfo> popupWidgets;
};

#endif // POPUPCONTAINERWIDGET_H
