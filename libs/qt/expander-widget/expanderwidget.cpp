#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>

#include "expanderwidget.h"

ExpanderWidget::ExpanderWidget(QWidget *parent)
    : QWidget(parent)
{
    button = new QPushButton();
    button->setObjectName("__qt__passive_button");
    button->setText("Expander");
    button->setIcon(QIcon(":/arrow-expanded.png"));
	button->setFlat(true);
	button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	button->setStyleSheet("QPushButton {text-align: left; font-weight: bold; border: none;} QPushButton:hover {color: blue;}");

    connect(button, SIGNAL(clicked()), this, SLOT(buttonPressed()));

    stackWidget = new QStackedWidget();

    layout = new QVBoxLayout();
    layout->addWidget(button, 0, Qt::AlignTop);
    layout->addWidget(stackWidget);
    setLayout(layout);
}

void ExpanderWidget::buttonPressed() {

	if (expanded) {
        expanded = false;
        button->setIcon(QIcon(":/arrow.png"));

        QSize size = layout->sizeHint();
        int width = size.width();
//        int height = size.height();

		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        stackWidget->hide();
        resize(width, 20);

        updateGeometry();
	} else {
        expanded = true;
        button->setIcon(QIcon(":/arrow-expanded.png"));

		// in certain situations QSizePolicy::Preferred could be
		// preferrable as a vertical policy.
		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        stackWidget->show();

        updateGeometry();
    }

    emit expanderChanged(expanded);
}

QSize ExpanderWidget::sizeHint() const {
    return QSize(200, 20);
}

void ExpanderWidget::setWidget(QWidget *widget) {
	QWidget* existingWidget = stackWidget->widget(0);

	if (existingWidget) {
		stackWidget->removeWidget(existingWidget);
		existingWidget->deleteLater();
	}

	stackWidget->addWidget(widget);
}


QWidget* ExpanderWidget::widget(void) const {
	return stackWidget->widget(0);
}

QString ExpanderWidget::expanderTitle() const {
	return button->text();
}

void ExpanderWidget::setExpanderTitle(QString const &newTitle) {
    button->setText(newTitle);
}

bool ExpanderWidget::isExpanded() const {
	return expanded;
}

void ExpanderWidget::setExpanded(bool flag) {
	if (flag != expanded) {
		buttonPressed();
	}
}

