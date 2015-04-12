#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <array>
#include <tuple>
#include <string>
#include <deque>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <tina++/tina.h>
#include <QTimer>
#include <QByteArray>

#include "basefrontend.h"

class QSettings;
class QSortFilterProxyModel;
class QSignalMapper;
class TinaInterface;
class QTableView;

////////////////////////////////////////////////////////////////////////////////
// StreamModel

class StreamModel : public QAbstractTableModel {
  Q_OBJECT

public:
  enum Column {
    COLUMN_TIME,
    COLUMN_SOURCE,
    COLUMN_ICON,
    COLUMN_MESSAGE,

    COLUMN_MAX
  };

  enum DataColumn {
    DATA_ICON,
    DATA_MESSAGE,
    DATA_SOURCE,
    DATA_TIME
  };

  enum Icon {
    ICON_INFO,
    ICON_WARNING,
    ICON_CRITICAL,
    ICON_ERROR,

    ICON_MAX
  };

  static const int FilterRole = Qt::UserRole + 1;

  typedef unsigned SourceId;
  typedef float Time;
  typedef std::tuple<Icon, QString, SourceId, Time> Row;
  typedef std::deque<Row> Rows;

  StreamModel(QObject* parent = 0);

  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;

  bool insertRow(char level, const char* data, std::size_t len, unsigned source);

  const Rows& rows() const { return rows_; }

  void clear();

  void setLogSource(char source, const QString&& name);
  const std::array<QString, 127>& getLogSources() const { return log_sources_; }

private slots:
  void insertRowsTimeout(void);

private:
  Rows row_buffer_;
  Rows rows_;
  int last_size;
  std::array<QString, 127> log_sources_;
  Time logtime_;

  QTimer insertTimer;

  int old_size;

};

////////////////////////////////////////////////////////////////////////////////
// LogFilter

class LogFilter : public QSortFilterProxyModel {
public:
  LogFilter(QObject *parent = 0);

  std::string getFilterSource() const { return filter_source_; }
  void setFilterSource(const std::string&& filter);

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
  std::string filter_source_;
};

////////////////////////////////////////////////////////////////////////////////
// LogView

class LogView : public BaseFrontend {
    Q_OBJECT
    NOT_COPYABLE(LogView);
    NOT_MOVABLE(LogView);

public:
  explicit
  LogView(TinaInterface* interface, QWidget *parent = 0);

  ~LogView();

  template<class... Args>
  void insertRow(Args&&... args) {
	log_model_->insertRow(std::forward<Args>(args)...);
  }

  void setScrollOnOutput(bool on);
  void activate(QIODevice* stream);
  void deactivate();
  void onStart();

  void setLogSource(char source, const QString&& name) {
      log_model_->setLogSource(source, std::move(name));
  }

  void readSettings();
  void writeSettings();

public slots:
  void onConnected(bool readOnly, bool isBuffered, QIODevice*) override;
  void onDisconnected(bool reconnecting) override;
  void clear() override;
  void writeData(QByteArray data) override;

protected:
  void keyPressEvent ( QKeyEvent * e );

private slots:
  void writeLine(QByteArray line);
  void scroll(int, int);
  void activated(QModelIndex index);
  void contextMenu(QPoint);
  void copy();
  void hideMsgsFromSource();
  void filterSrc(int index);
  void deactivateFilter();
  void activateFilter();
  void onSendTimeout(void);


signals:
  void activatedGraph(int index);

private:
  QTableView* log_;
  StreamModel* log_model_;
  LogFilter* filter_;
  QSignalMapper* filter_mapper_;

  bool scroll_on_output_;

  QTimer sendTimer;
  QByteArray timedSendString;

  bool hasReadOnlyConnection;
};

#endif // LOGVIEW_H
