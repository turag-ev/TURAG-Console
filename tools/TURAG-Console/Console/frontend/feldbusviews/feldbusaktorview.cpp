#include "feldbusaktorview.h"
#include "frontend/graphutils/datagraph.h"
#include <libs/lineeditext.h>
#include <libs/buttongroupext.h>
#include <libs/splitterext.h>

#include <QCheckBox>
#include <QColor>
#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QMessageBox>
#include <QPalette>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalMapper>
#include <QSplitter>
#include <QStringList>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#include <vector>


FeldbusAktorView::FeldbusAktorView(Aktor *aktor, QWidget *parent) :
    QWidget(parent), actor(aktor), commandset(nullptr)
{
    QIntValidator* intervalValidator = new QIntValidator(1, 100000, this);
    QIntValidator* lengthValidator = new QIntValidator(1, 100000, this);

	QHBoxLayout* hlayout = new QHBoxLayout;
	SplitterExt* main_splitter = new SplitterExt("FeldbusAktorView_main_splitter");
    QVBoxLayout* left_layout = new QVBoxLayout;
    QVBoxLayout* right_layout = new QVBoxLayout;
	QWidget* right_layout_holder = new QWidget;
    QGridLayout* value_grid = new QGridLayout;
    QVBoxLayout* settings_layout = new QVBoxLayout;
	QSplitter* left_splitter = new QSplitter;
	QWidget* left_layout_holder = new QWidget;

	left_splitter->setOrientation(Qt::Vertical);
	left_splitter->addWidget(left_layout_holder);
	left_layout_holder->setLayout(left_layout);

	hlayout->addWidget(main_splitter);
	main_splitter->addWidget(left_splitter);
	main_splitter->addWidget(right_layout_holder);
	main_splitter->setChildrenCollapsible(false);
	main_splitter->setStretchFactor(1, 2);
	main_splitter->restoreState();
	right_layout_holder->setLayout(right_layout);
	setLayout(hlayout);

	updateDeviceValues = new QPushButton("Werte aktualisieren/zurücksetzen");
    connect(updateDeviceValues, SIGNAL(clicked()), this, SLOT(onUpdateDeviceValues()));

    QFrame* scrollframe = new QFrame;
    scrollframe->setLayout(value_grid);

    QScrollArea* scrollarea = new QScrollArea;
    scrollarea->setWidgetResizable(true);
    scrollarea->setWidget(scrollframe);

    left_layout->addWidget(scrollarea);
    QVBoxLayout* left_sub_layout = new QVBoxLayout;
    left_layout->addLayout(left_sub_layout);
    left_sub_layout->addWidget(updateDeviceValues);
    QLabel* descr = new QLabel("Markierte Werte werden in der Diagrammanzeige berücksichtigt");
    descr->setWordWrap(true);
    left_sub_layout->addWidget(descr);
    left_layout->setAlignment(left_sub_layout, Qt::AlignBottom);


    plot = new DataGraph;
    right_layout->addWidget(plot);
    right_layout->addLayout(settings_layout);
	plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    startStopDataUpdate = new QPushButton("Start");
    startStopDataUpdate->setDisabled(true);
    connect(startStopDataUpdate, SIGNAL(clicked()), this, SLOT(onStartStopDataUpdate()));
    updateDuration = new QLabel("");

    QHBoxLayout* radio_layout = new QHBoxLayout;
    settings_layout->addLayout(radio_layout);
    cyclicDataUpdate = new QRadioButton("zyklische Datenaktualisierung");
	cyclicDataUpdate->setChecked(true);
	cyclicDataUpdate->setToolTip("Daten werden ständig vom Gerät geholt und angezeigt");
    radio_layout->addWidget(cyclicDataUpdate);
    oneShotDataUpdate = new QRadioButton("einmalige Datenaktualisierung");
	oneShotDataUpdate->setToolTip("Die Änderung eines Wertes startet die Datenaufzeichnung für die angegebene Dauer");
    radio_layout->addWidget(oneShotDataUpdate);
	radio_layout->addWidget(startStopDataUpdate);
	ButtonGroupExt* dataUpdateButtonGroup = new ButtonGroupExt("feldbusAktorViewDataUpdateButtonGroup", this);
    dataUpdateButtonGroup->addButton(cyclicDataUpdate);
    dataUpdateButtonGroup->addButton(oneShotDataUpdate);
    dataUpdateButtonGroup->readSettings();
	settings_layout->addWidget(updateDuration);
	
    QFormLayout* settings_form = new QFormLayout;
    settings_layout->addLayout(settings_form);
    QLabel* label1 = new QLabel("Abfrage-Intervall [ms]");
    QLabel* label2 = new QLabel("Aufzeichnungsdauer [Samples]");
    updateInterval = new LineEditExt("FeldbusAktorViewUpdateInterval", "10");
    updateInterval->setValidator(intervalValidator);
    connect(updateInterval, SIGNAL(textEdited(QString)), this, SLOT(onInputEdited()));
    updateLength = new LineEditExt("FeldbusAktorViewUpdateLength", "100");
    updateLength->setValidator(lengthValidator);
    connect(updateLength, SIGNAL(textEdited(QString)), this, SLOT(onInputEdited()));
    settings_form->addRow(label1, updateInterval);
    settings_form->addRow(label2, updateLength);

	updateTimer = new QTimer(this);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

	scriptExecTimer = new QTimer(this);
	scriptExecTimer->setSingleShot(true);
	connect(scriptExecTimer, SIGNAL(timeout()), this, SLOT(executeNextScriptCommand()));

	setMapper = new QSignalMapper(this);
	connect(setMapper, SIGNAL(mapped(int)), this, SLOT(onValueSet(int)));
	addScriptSnippetMapper = new QSignalMapper(this);
	connect(addScriptSnippetMapper, SIGNAL(mapped(int)), this, SLOT(addScriptSnippet(int)));

	QVBoxLayout* script_vlayout = new QVBoxLayout;
	QWidget* script_vlayout_holder = new QWidget;
	script_vlayout_holder->setLayout(script_vlayout);
	left_splitter->addWidget(script_vlayout_holder);

	left_splitter->setSizes({{4,1}});

	scriptGroupBox = new QGroupBox("Skript-Editor");
	script_vlayout->addWidget(scriptGroupBox);

	QVBoxLayout* script_editor_layout = new QVBoxLayout;
	QHBoxLayout* script_button_layout = new QHBoxLayout;
	script_editor_layout->addLayout(script_button_layout);
	scriptGroupBox->setLayout(script_editor_layout);

	scriptEditor = new QTextEdit;
	script_editor_layout->addWidget(scriptEditor);
	runScriptButton = new QPushButton("Skript ausführen");
	script_vlayout->addWidget(runScriptButton);
	connect(runScriptButton, SIGNAL(pressed()), this, SLOT(executeScript()));
	stopScriptButton = new QPushButton("STOP");
	stopScriptButton->setVisible(false);
	script_vlayout->addWidget(stopScriptButton);
	connect(stopScriptButton, SIGNAL(pressed()), this, SLOT(stopScript()));

	QPushButton* insertWaitBtn = new QPushButton("wait( x )");
	insertWaitBtn->setToolTip("Fügt einen \"wait( x )\"-Befehl ein, der die Befehlsausführung um <x> Sekunden verzögert.");
	script_button_layout->addWidget(insertWaitBtn);
	connect(insertWaitBtn, &QPushButton::pressed, [this] () {
		scriptEditor->moveCursor(QTextCursor::StartOfLine);
		scriptEditor->moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		if (!scriptEditor->textCursor().selectedText().simplified().isEmpty()) {
			scriptEditor->moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
			scriptEditor->insertPlainText("\n");
		}
		scriptEditor->moveCursor(QTextCursor::EndOfLine);
		scriptEditor->insertPlainText("wait( 1 );");
	});

	QPushButton* insertCaptureBtn = new QPushButton("startCapture()");
	insertCaptureBtn->setToolTip("Fügt einen \"startCapture()\"-Befehl ein, der die Datenaufzeichnung startet, sofern  \"einmalige Datenaktualisierung\" gewählt ist.");
	script_button_layout->addWidget(insertCaptureBtn);
	connect(insertCaptureBtn, &QPushButton::pressed, [this] () {
		scriptEditor->moveCursor(QTextCursor::StartOfLine);
		scriptEditor->moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		if (!scriptEditor->textCursor().selectedText().simplified().isEmpty()) {
			scriptEditor->moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
			scriptEditor->insertPlainText("\n");
		}
		scriptEditor->moveCursor(QTextCursor::EndOfLine);
		scriptEditor->insertPlainText("startCapture();");
	});


    onInputEdited();


    /*
     * read command set
     */
    if (!actor) return;

    if (commandset) {
        if (updateTimer->isActive()) {
            onStartStopDataUpdate();
        }

        delete[] commandset;
        plot->clear();

        for (CommandsetEntry& entry : commandsetGrid) {
            if (entry.button) {
                entry.button->disconnect();
                entry.button->deleteLater();
            }
            entry.checkbox->disconnect();
            entry.checkbox->deleteLater();
            entry.label->deleteLater();
            entry.value->deleteLater();
        }
        commandsetGrid.clear();
    }

    unsigned commandsetLength = actor->getCommandsetLength();
    if (commandsetLength == 0) {
        return;
    }

    commandset = new Aktor::Command_t[commandsetLength];
    if (!actor->populateCommandSet(commandset, commandsetLength)) {
        return;
    }

    char command_name[256];

	unsigned i = 0;
	for (i = 0; i < commandsetLength; ++i) {
		if (commandset[i].length == Aktor::Command_t::CommandLength::text) {
			// text entry command
			QLabel* text = new QLabel;

			if (actor->getCommandName(i+1, command_name)) {
				text->setText(QString("<b>%1</b>").arg(command_name));
			} else {
				text->setText("<b>\?\?\?</b>");
			}

			value_grid->addWidget(text, i, 0, 1, -1, Qt::AlignBottom);
			value_grid->setRowMinimumHeight(i, 25);
		} else if (commandset[i].length != Aktor::Command_t::CommandLength::none) {
			// actual command entry
            CommandsetEntry entry;
            entry.key = i+1;
            entry.label = new QLabel;

            if (actor->getCommandName(entry.key, command_name)) {
				entry.caption = QString(command_name);
				entry.label->setText(entry.caption);
            } else {
				entry.label->setText("???");
            }

            entry.value = new LineEditExt;
            if (commandset[i].writeAccess == Aktor::Command_t::WriteAccess::read_only) {
                entry.value->setReadOnly(true);
                QPalette pal = entry.value->palette();
                QColor clr = this->palette().color(QPalette::Window);
                pal.setColor(QPalette::Active, QPalette::Base, clr);
                pal.setColor(QPalette::Inactive, QPalette::Base, clr);
                entry.value->setPalette(pal);
                entry.button = nullptr;
            } else {
                entry.button = new QPushButton("Set");
                connect(entry.button, SIGNAL(clicked()), setMapper, SLOT(map()));
                setMapper->setMapping(entry.button, commandsetGrid.size());
                connect(entry.value, SIGNAL(returnPressed()), setMapper, SLOT(map()));
                setMapper->setMapping(entry.value, commandsetGrid.size());
                connect(entry.value, SIGNAL(textEdited(QString)), this, SLOT(onUserInput()));

				if (!entry.caption.isEmpty()) {
					connect(entry.label, SIGNAL(linkActivated(QString)), addScriptSnippetMapper, SLOT(map()));
					addScriptSnippetMapper->setMapping(entry.label, commandsetGrid.size());
					entry.label->setText(QString("<a href=\"http://www.turag.de\">%1</a>").arg(entry.caption));
				}
            }

            entry.checkbox = new QCheckBox;
            connect(entry.checkbox, SIGNAL(stateChanged(int)), this, SLOT(onCheckboxChanged()));

			value_grid->addWidget(entry.label, i, 0, Qt::AlignVCenter);
			value_grid->addWidget(entry.value, i, 1, Qt::AlignVCenter);
            if (entry.button) {
				value_grid->addWidget(entry.button, i, 2, Qt::AlignVCenter);
            }
			value_grid->addWidget(entry.checkbox, i, 3, Qt::AlignVCenter);

            commandsetGrid.append(entry);
		}
	}
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	value_grid->addWidget(spacer, i, 0);

    onUpdateDeviceValues();
}


void FeldbusAktorView::onUpdateDeviceValues(void) {
    for (CommandsetEntry& entry : commandsetGrid) {
        if (commandset[entry.key-1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
            int32_t value;
            if (!actor->getValue(entry.key, &value)) {
                entry.value->setText("ERROR");
            } else {
                entry.value->setText(QString("%1").arg(value));
            }
        } else {
            float value;
            if (!actor->getValue(entry.key, &value)) {
                entry.value->setText("ERROR");
            } else {
                entry.value->setText(QString("%1").arg(value));
            }
        }

        if (commandset[entry.key-1].writeAccess == Aktor::Command_t::WriteAccess::write_only) {
            QPalette pal = entry.value->palette();
            pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
            entry.value->setPalette(pal);
        }
    }
}


void FeldbusAktorView::onStartStopDataUpdate(void) {
    validateInput();

    if (!updateTimer->isActive() && startStopDataUpdate->isEnabled()) {
        startStopDataUpdate->setText("Stop");
        updateTimer->setInterval(updateInterval->text().toUInt());
        updateCounter = 0;
        plot->clear();
        disableCheckboxes();
        cyclicDataUpdate->setDisabled(true);
        oneShotDataUpdate->setDisabled(true);

        std::vector<uint8_t> outputTable;

        for (CommandsetEntry& entry : commandsetGrid) {
            if (entry.checkbox->isChecked()) {
                outputTable.push_back(entry.key);
                plot->addChannel(entry.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            }
        }
        actor->setStructuredOutputTable(outputTable);

        updateStartTime.start();
        updateTimer->start(updateInterval->text().toInt());
        onTimeout();
    } else {
        startStopDataUpdate->setText("Start");
        updateTimer->stop();
        enableCheckboxes();
        cyclicDataUpdate->setDisabled(false);
        oneShotDataUpdate->setDisabled(false);
    }

}

FeldbusAktorView::~FeldbusAktorView(void) {
    if (commandset) {
        if (updateTimer->isActive()) {
            onStartStopDataUpdate();
        }
        delete[] commandset;
    }
}


void FeldbusAktorView::validateInput(void) {
    if (!updateInterval->hasAcceptableInput()) {
        updateInterval->setText("10");
    }
    if (!updateLength->hasAcceptableInput()) {
        updateLength->setText("100");
	}
}

void FeldbusAktorView::errorPrompt(const QString &msg)
{
	QMessageBox msgBox;
	msgBox.setText(msg);
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.exec();
}


void FeldbusAktorView::onInputEdited(void) {
    if (updateInterval->hasAcceptableInput() && updateLength->hasAcceptableInput()) {
        updateDuration->setText(QString("Aufzeichnungsfenster: %1 s").arg(updateInterval->text().toFloat() * updateLength->text().toFloat() / 1000.0f));
    } else {
        updateDuration->setText("");
    }

}

void FeldbusAktorView::onTimeout(void) {
    if (oneShotDataUpdate->isChecked()) {
        ++updateCounter;

        if (updateCounter > updateLength->text().toUInt()) {
            onStartStopDataUpdate();
        }
    }

    int msecs = updateStartTime.elapsed();

    std::vector<Aktor::StructuredDataPair_t> output;
    actor->getStructuredOutput(&output);

    int channel = 0;
	for (Aktor::StructuredDataPair_t& data_ : output) {
		plot->addData(channel, QPointF(msecs, data_.value));
        ++channel;
    }

}


void FeldbusAktorView::onValueSet(int id) {
    uint8_t key = commandsetGrid.at(id).key;

    if (commandset[key - 1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
        actor->setValue(key, static_cast<int32_t>(commandsetGrid.at(id).value->text().toInt()));

        int32_t value;
        if (!actor->getValue(key, &value)) {
            commandsetGrid.at(id).value->setText("ERROR");
        } else {
            commandsetGrid.at(id).value->setText(QString("%1").arg(value));
        }
    } else {
        actor->setValue(key, commandsetGrid.at(id).value->text().toFloat());

        float value;
        if (!actor->getValue(key, &value)) {
            commandsetGrid.at(id).value->setText("ERROR");
        } else {
            commandsetGrid.at(id).value->setText(QString("%1").arg(value));
        }
    }

    if (oneShotDataUpdate->isChecked() && !updateTimer->isActive()) {
        onStartStopDataUpdate();
    }

    QPalette pal = commandsetGrid.at(id).value->palette();
    pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
    commandsetGrid.at(id).value->setPalette(pal);
}

void FeldbusAktorView::addScriptSnippet(int id)
{
	QString snippet = QString("set(\"%1\", %2);").arg(commandsetGrid.at(id).caption).arg(commandsetGrid.at(id).value->text());
	scriptEditor->moveCursor(QTextCursor::StartOfLine);
	scriptEditor->moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	if (!scriptEditor->textCursor().selectedText().simplified().isEmpty()) {
		scriptEditor->moveCursor(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
		scriptEditor->insertPlainText("\n");
	}
	scriptEditor->moveCursor(QTextCursor::EndOfLine);
	scriptEditor->insertPlainText(snippet);
}

void FeldbusAktorView::executeScript()
{
	currentScriptEntry = 0;
	scriptCommandList.clear();
	ScriptEntry entry;

	QString script = scriptEditor->toPlainText();
	QStringList commands = script.trimmed().split(QChar(';'), QString::SkipEmptyParts);

	int line = 1;
	for (QString command : commands) {
		command = command.trimmed();

		if (command.startsWith("set")) {
			entry.commandEntry = command;
			command = command.remove(0, 3).trimmed();
			if (!(command.startsWith('(') && command.endsWith(')'))) {
				errorPrompt(QString("fehlende runde Klammern (Zeile %1)").arg(line));
				return;
			}
			command = command.mid(1, command.size()-2).trimmed();
			int quoteEnd = command.lastIndexOf('"');

			if (!command.startsWith('"') || quoteEnd == 0) {
				errorPrompt(QString("fehlende Anführungszeichen (Zeile %1)").arg(line));
				return;
			}
			QString setCaption = command.mid(1, quoteEnd-1);
			QList<CommandsetEntry>::ConstIterator found = std::find_if(commandsetGrid.constBegin(), commandsetGrid.constEnd(), [&] (const CommandsetEntry& e) {
				if (setCaption == e.caption) {
					return true;
				} else {
					return false;
				}
			});
			if (found == commandsetGrid.constEnd()) {
				errorPrompt(QString("command key \"%1\" für \"set\" nicht vorhanden (Zeile %2)").arg(setCaption).arg(line));
				return;
			}

			command = command.right(command.size() - quoteEnd - 1).trimmed();
			if (!command.startsWith(',')) {
				errorPrompt(QString("fehlendes Komma (Zeile %1)").arg(line));
				return;
			}

			command = command.right(command.size() - 1).trimmed();

			if (commandset[found->key - 1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
				bool ok = false;
				int value = command.toInt(&ok);

				if (!ok) {
					errorPrompt(QString("Argument für command key \"%1\" muss Ganzzahl sein (Zeile %2)").arg(setCaption).arg(line));
					return;
				}
				entry.intValue = value;
			} else {
				bool ok = false;
				float value = command.toFloat(&ok);

				if (!ok) {
					errorPrompt(QString("Argument für command key \"%1\" muss Fließkommazahl sein (Zeile %2)").arg(setCaption).arg(line));
					return;
				}
				entry.floatValue = value;
			}
			entry.key = found->key;

		} else if (command.startsWith("wait")) {
			entry.commandEntry = command;
			command = command.remove(0, 4).trimmed();
			if (!(command.startsWith('(') && command.endsWith(')'))) {
				errorPrompt(QString("fehlende runde Klammern (Zeile %1)").arg(line));
				return;
			}
			command = command.mid(1, command.size()-2).trimmed();

			bool ok = false;
			float value = command.toFloat(&ok);
			if (!ok || value < 0.0f) {
				errorPrompt(QString("Argument von wait muss pos. Fließkommazahl sein (Zeile %1)").arg(line));
				return;
			}

			entry.key = ScriptCommand::wait;
			entry.floatValue = value;
		} else if (command.startsWith("startCapture")) {
			entry.commandEntry = command;
			command = command.remove(0, 12).trimmed();
			if (!(command.startsWith('(') && command.endsWith(')'))) {
				errorPrompt(QString("fehlende runde Klammern (Zeile %1)").arg(line));
				return;
			}
			command = command.mid(1, command.size()-2).trimmed();
			if (!command.isEmpty()) {
				errorPrompt(QString("Unerwartetes Argument (Zeile %1)").arg(line));
				return;
			}
			entry.key = ScriptCommand::startCapture;
		} else {
			errorPrompt(QString("Unbekanntes Kommando \"%1\" (Zeile %2)").arg(command).arg(line));
			return;
		}

		scriptCommandList.append(entry);
	}

	qDebug() << scriptCommandList.size() << "Skripteinträge";

	if (scriptCommandList.size() > 0) {
		runScriptButton->setVisible(false);
		stopScriptButton->setVisible(true);
		scriptGroupBox->setEnabled(false);

		scriptExecTimer->start(0);
	}

}

void FeldbusAktorView::stopScript()
{
	stopScriptButton->setVisible(false);
	runScriptButton->setVisible(true);
	scriptGroupBox->setEnabled(true);
	scriptExecTimer->stop();
}

void FeldbusAktorView::executeNextScriptCommand()
{
	if (currentScriptEntry < scriptCommandList.size()) {
		ScriptEntry entry = scriptCommandList.at(currentScriptEntry);
		int delay = 0;

		qDebug() << "execute" << entry.commandEntry;

		if (entry.key == ScriptCommand::wait) {
			delay = static_cast<int>(entry.floatValue * 1000);
		} else if (entry.key == ScriptCommand::startCapture) {
			if (oneShotDataUpdate->isChecked() && !updateTimer->isActive()) {
				onStartStopDataUpdate();
			}
		} else {
			if (commandset[entry.key - 1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
				actor->setValue(static_cast<uint8_t>(entry.key), static_cast<int32_t>(entry.intValue));
			} else {
				actor->setValue(static_cast<uint8_t>(entry.key), entry.floatValue);
			}
		}


		scriptExecTimer->start(delay);
		++currentScriptEntry;
	} else {
		qDebug() << "Script execution finished";
		stopScript();
	}
}


void FeldbusAktorView::onCheckboxChanged(void) {
    unsigned int numberOfCheckedValues = 0;
    for (CommandsetEntry& entry : commandsetGrid) {
        if (entry.checkbox->isChecked()) {
            ++numberOfCheckedValues;
        }
    }

    if (numberOfCheckedValues > 0 && static_cast<int>(numberOfCheckedValues) <= actor->getStructuredOutputTableLength()) {
        startStopDataUpdate->setEnabled(true);
    } else {
        startStopDataUpdate->setDisabled(true);
    }
}

void FeldbusAktorView::enableCheckboxes(void) {
    for (CommandsetEntry& entry : commandsetGrid) {
        entry.checkbox->setEnabled(true);
    }
}

void FeldbusAktorView::disableCheckboxes(void) {
    for (CommandsetEntry& entry : commandsetGrid) {
        entry.checkbox->setEnabled(false);
    }
}

void FeldbusAktorView::onUserInput(void) {
    QWidget* widget = static_cast<QWidget*>(sender());

    QPalette pal = widget->palette();
    pal.setColor(QPalette::Active, QPalette::Base, Qt::red);
	widget->setPalette(pal);
}

