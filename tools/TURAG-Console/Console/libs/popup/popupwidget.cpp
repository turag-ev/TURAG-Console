#include "popupwidget.h"

#include <QSettings>


PopupWidget::PopupWidget(QWidget *mainWidget_, QSize defaultPreferredSize_, QString saveGeometryIdentifier_) :
	ResizableFrame(mainWidget_, false, nullptr),
	saveGeometryIdentifier(saveGeometryIdentifier_)
{
	// load default preferred Size
	if (defaultPreferredSize_.isEmpty()) {
		preferredSize = QSize(defaultPreferredWidth, defaultPreferredHeight);
	} else {
		preferredSize = defaultPreferredSize_;
	}

	// try to load preferred size from settings storage
	if (!saveGeometryIdentifier.isEmpty()) {
		QSettings settings;
		settings.beginGroup(QStringLiteral("popupWidgetGeometry"));
		preferredSize = settings.value(saveGeometryIdentifier, preferredSize).toSize();
	}


	setAutoFillBackground(true);

	connect(this, &ResizableFrame::geometryChanged, [this]() {
		preferredSize = size();
	});
}

void PopupWidget::hideEvent(QHideEvent *) {
	if (!saveGeometryIdentifier.isEmpty()) {
		QSettings settings;
		settings.beginGroup(QStringLiteral("popupWidgetGeometry"));
		settings.setValue(saveGeometryIdentifier, preferredSize);
	}
}


