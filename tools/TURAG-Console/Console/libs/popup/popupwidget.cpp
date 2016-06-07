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
        correctPreferredSize();
    }


	setAutoFillBackground(true);

	connect(this, &ResizableFrame::geometryChanged, [this]() {
		preferredSize = size();

        correctPreferredSize();
        if (preferredSize != size()) {
            resize(preferredSize);
        }
    });
}

void PopupWidget::hideEvent(QHideEvent *) {
	if (!saveGeometryIdentifier.isEmpty()) {
		QSettings settings;
		settings.beginGroup(QStringLiteral("popupWidgetGeometry"));
		settings.setValue(saveGeometryIdentifier, preferredSize);
    }
}

void PopupWidget::correctPreferredSize()
{
    if (preferredSize.width() < minPreferredWidth) {
        preferredSize.setWidth(minPreferredWidth);
    }
    if (preferredSize.height() < minPreferredHeight) {
        preferredSize.setHeight(minPreferredHeight);
    }
}


