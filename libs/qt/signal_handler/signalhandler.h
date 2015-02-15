#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <QObject>


/**
 * @brief Behandlet Signale in Qt-Konsolenanwendungen.
 *
 * Wird eine Qt-Konsolenanwendung mit Strg+C beendet, so wird sie hart
 * gestoppt und Destruktoren werden nicht mehr aufgerufen. Um das zu
 * verhindern, müssen die Signale des Betriebssystems abgefangen und verarbeitet
 * werden.
 *
 * Diese Klasse kann benutzt werden, um in Qt auf komfortable Art und Weise
 * Signale des Betriebssystems zu verarbeiten. Die SignalHandler-Klasse
 * ist ein Singleton, die nach Programmstart initialisiert werden muss. Dann
 * können die Signale mit einem beliebigen Slot im Programm verbunden werden.
 *
 * Beispiel:
 * \code
 *  SignalHandler::init();
 *  QObject::connect(SignalHandler::get(), SIGNAL(sigInt()), &a, SLOT(quit()));
 *  QObject::connect(SignalHandler::get(), SIGNAL(sigKill()), &a, SLOT(quit()));
 *  QObject::connect(SignalHandler::get(), SIGNAL(sigTerm()), &a, SLOT(quit()));
 * \endcode
 */
class SignalHandler : public QObject
{
    Q_OBJECT
public:
    static void init(void);
    static SignalHandler* get(void);

public slots:
    void emitSigKill(void);
    void emitSigInt(void);
    void emitSigTerm(void);

signals:
    void sigKill(void);
    void sigInt(void);
    void sigTerm(void);

private:
    explicit SignalHandler(QObject *parent = 0) : QObject(parent) { }

};

#endif // SIGNALHANDLER_H
