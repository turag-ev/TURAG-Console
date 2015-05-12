#ifndef CHECKACTIONEXT_H
#define CHECKACTIONEXT_H

#include <QAction>
#include <QSettings>

class CheckActionExt : public QAction
{
    Q_OBJECT
public:
	explicit CheckActionExt(const QString& identifier, const QString &text, bool standardValue, QObject* parent, bool persistent_ = true) :
		QAction(text, parent), persistent(persistent_)
    {
        setCheckable(true);

		if (!identifier.isEmpty() && persistent) {
            setObjectName(identifier);
            QSettings settings;
            setChecked(settings.value(identifier, standardValue).toBool());
            connect(this, SIGNAL(triggered(bool)), this, SLOT(saveToggled()));
        } else {
            setChecked(standardValue);
        }
    }

    ~CheckActionExt(void) {
        if (!objectName().isEmpty()) {
		   // not necessary...

			// saveToggled();
        }
    }

signals:

protected slots:
    void saveToggled(void) {
		if (persistent) {
			QSettings settings;
			settings.setValue(objectName(), isChecked());
		}
    }

private:
	bool persistent;

};

#endif // CHECKACTIONEXT_H
