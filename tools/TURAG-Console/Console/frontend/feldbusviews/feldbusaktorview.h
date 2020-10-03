#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSAKTORVIEW_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSAKTORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/legacystellantriebedevice.h>
#include <frontend/graphutils/datagraph.h>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QList>
#include <QString>
#include <QTime>
#include <QElapsedTimer>


class QGridLayout;
class QGroupBox;
class QIntValidator;
class QRadioButton;
class QSignalMapper;
class QTextEdit;
class QTimer;

class LineEditExt;


using namespace TURAG::Feldbus; // FIXME: evil!



class FeldbusAktorView : public QWidget
{
    Q_OBJECT

public:
    explicit FeldbusAktorView(LegacyStellantriebeDevice* aktor, QWidget *parent = 0);
    ~FeldbusAktorView(void);

protected slots:
    void onUpdateDeviceValues(void);
    void onStartStopDataUpdate(void);
    void onInputEdited(void);
    void onTimeout(void);
    void onValueSet(int id);
    void onCheckboxChanged(void);
    void onUserInput(void);
	void addScriptSnippet(int id);

	void executeScript(void);
	void stopScript(void);
	void executeNextScriptCommand(void);

protected:
    void enableCheckboxes(void);
    void disableCheckboxes(void);

    unsigned updateCounter;
	
private:
    void validateInput(void);
	void errorPrompt(const QString& msg);

	
protected:
    struct CommandsetEntry {
		QString caption;
        uint8_t key;
        QLabel* label;
        QLineEdit* value;
        QPushButton* button;
        QCheckBox* checkbox;
    };

	struct ScriptEntry {
		QString commandEntry;
		int key;
		int intValue;
		float floatValue;
	};

	enum ScriptCommand {
		wait, startCapture
	};

    LegacyStellantriebeDevice* actor;

    DataGraph* plot;
    QPushButton* getCommandSet;
    QPushButton* updateDeviceValues;
    QRadioButton* cyclicDataUpdate;
    QRadioButton* oneShotDataUpdate;
    QPushButton* startStopDataUpdate;
    LineEditExt* updateInterval;
    LineEditExt* updateLength;
    QLabel* updateDuration;
    QTimer* updateTimer;
    QList<CommandsetEntry> commandsetGrid;
    LegacyStellantriebeDevice::Command_t* commandset;
    QSignalMapper* setMapper;
	QSignalMapper* addScriptSnippetMapper;
    QElapsedTimer updateStartTime;

	QTextEdit* scriptEditor;
	QTimer* scriptExecTimer;
	QList<ScriptEntry> scriptCommandList;
	int currentScriptEntry;
	QPushButton* stopScriptButton;
	QPushButton* runScriptButton;
	QGroupBox* scriptGroupBox;

};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSAKTORVIEW_H
