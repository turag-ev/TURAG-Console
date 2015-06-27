#ifndef TINAINTERFACE_H
#define TINAINTERFACE_H

#include <QObject>
#include <vector>

class TinaInterface : public QObject
{
    Q_OBJECT

public:
    explicit TinaInterface(QObject *parent = 0);

signals:
    void tinaPackageReady(QByteArray line);
    void cmenuDataReady(QByteArray data);

public slots:
    void dataInput(const QByteArray data);
    void clear(void);

protected:
    unsigned tina_package_depth_;
    std::vector<QByteArray> packageBuffer_;

	static QByteArray trimmedBuffer(const char *begin, const char *end);
    static QByteArray trimmedBuffer(const QByteArray& data) {
		return trimmedBuffer(data.constBegin(), data.constEnd());
    }
};

#endif // TINAINTERFACE_H
