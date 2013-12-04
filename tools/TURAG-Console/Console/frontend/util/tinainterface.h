#ifndef TINAINTERFACE_H
#define TINAINTERFACE_H

#include <QObject>

class TinaInterface : public QObject
{
    Q_OBJECT

public:
    explicit TinaInterface(QObject *parent = 0);

signals:
    void beginUpdate(void);
    void tinaPackageReady(QByteArray line);
    void cmenuDataReady(QByteArray data);
    void endUpdate(void);

public slots:
    virtual void dataInput(const QByteArray data);

protected:
    enum class BufferContentType {
        TINA_DEBUG,
        CMENU
    };

    BufferContentType content_;
    QByteArray packageBuffer_;
    static QByteArray trimmedBuffer(const QByteArray& data, const char *begin,
                                    const char *end);

    static QByteArray trimmedBuffer(const QByteArray& data) {
        return trimmedBuffer(data, data.begin(), data.end());
    }
};

#endif // TINAINTERFACE_H
