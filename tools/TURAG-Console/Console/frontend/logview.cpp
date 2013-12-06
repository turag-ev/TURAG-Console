#include <QTableView>
#include <QSettings>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QScrollBar>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QSignalMapper>
#include <QApplication>
#include <QTextStream>
#include <algorithm>
#include <QVBoxLayout>
#include "util/tinainterface.h"

#include "logview.h"

static QIcon icons[StreamModel::ICON_MAX];

////////////////////////////////////////////////////////////////////////////////
// StreamModel

StreamModel::StreamModel(QObject* parent) :
  QAbstractTableModel(parent),
  rows_(),
  last_size(),
  log_sources_(),
  logtime_()
{
  log_sources_[';'] = "System";
}

int StreamModel::rowCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return rows_.size();
}

int StreamModel::columnCount(const QModelIndex& parent) const {
  Q_UNUSED(parent);
  return COLUMN_MAX;
}

QVariant StreamModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (index.row() >= static_cast<int>(rows_.size()) || index.row() < 0)
    return QVariant();

  int column = index.column();
  int row = index.row();

  switch (role) {
  case Qt::DisplayRole:{
    Row data = rows_.at(row);

    switch (column) {
    case COLUMN_MESSAGE:
      return std::get<DATA_MESSAGE>(data);

    case COLUMN_SOURCE: {
     const QString& source = log_sources_[std::get<DATA_SOURCE>(data)];
      if (source.length() > 0) {
        return source;
      } else {
        return QChar::fromLatin1(std::get<DATA_SOURCE>(data));
      }
    }

    case COLUMN_TIME:
      return QString("%1").arg(std::get<DATA_TIME>(data), 0, 'f', 3);
    }
    break;
  }

  case Qt::DecorationRole:
    if (column == COLUMN_ICON) {
      return icons[std::get<DATA_ICON>(rows_.at(row))];
    }
    break;

  case FilterRole:
    if (column == COLUMN_SOURCE) {
      return std::get<DATA_SOURCE>(rows_.at(row));
    }
    break;

  case Qt::BackgroundRole:
    if (std::get<DATA_SOURCE>(rows_.at(row)) == ';') {
      return QColor(255, 128, 0);
    }
    break;

  case Qt::ForegroundRole:
    if (std::get<DATA_SOURCE>(rows_.at(row)) == ';') {
      return QColor(Qt::white);
    }
    break;
  }

  return QVariant();
}

QVariant StreamModel::headerData(int section,
                                 Qt::Orientation orientation,
                                 int role) const
{
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
      case COLUMN_MESSAGE:
        return "Nachricht";

      case COLUMN_SOURCE:
        return "Quelle";

      case COLUMN_ICON:
        return "";

      default:
        return QVariant();
      }
    }
  }

  return QVariant();
}

void StreamModel::beginUpdate() {
  last_size = rows_.size();
}

bool StreamModel::insertRow(char level, const char *data, std::size_t len, unsigned source)
{
  // icon
  Icon icon;
  switch (level) {
  case '-': icon = ICON_INFO;     break;
  case '?': icon = ICON_WARNING;  break;
  case '!': icon = ICON_ERROR;    break;
  default:  icon = ICON_INFO;     break;
  }

  if (source == ';' && strncmp(data, "SPIELZEIT:", std::min(len, (std::size_t)10))==0) {
    QTextStream stream(QByteArray(data, len));
    stream.seek(10); // "SPIELZEIT:"

    int sec = 0, msec = 0;
    stream >> sec;
    stream.seek(stream.pos() + 1); // ,
    stream >> msec;

    logtime_ = sec + msec / 1000.f;
    return true;
  }

  rows_.emplace_back(icon, QString::fromUtf8(data, len), source, logtime_);

  return true;
}

void StreamModel::endUpdate() {
  int size = rows_.size();
  int diff = size - last_size;
  if (diff == 0) {
    return;
  }
  int old_size = last_size;
/*
  while (diff > 250) {
    beginInsertRows(QModelIndex(), old_size, old_size + 250);
    endInsertRows();
    qApp->processEvents();
    old_size += 250;
    diff -= 250;
  }*/

  beginInsertRows(QModelIndex(), old_size, size - 1);
  endInsertRows();
}

void StreamModel::clear() {
  int size = rows_.size();

  beginRemoveRows(QModelIndex(), 0, size);

  rows_.clear();

  endRemoveRows();
}

void StreamModel::setLogSource(char source, const QString&& name) {
  log_sources_[static_cast<std::size_t>(source)] = std::move(name);
}

Qt::ItemFlags StreamModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return QAbstractTableModel::flags(index);
}

////////////////////////////////////////////////////////////////////////////////
// LogFilter

LogFilter::LogFilter(QObject *parent) :
  QSortFilterProxyModel(parent),
  filter_source_()
{
  setDynamicSortFilter(false);
}

void LogFilter::setFilterSource(const std::string&& filter) {
  filter_source_.assign(std::move(filter));
  invalidateFilter();
}

bool LogFilter::filterAcceptsRow(int sourceRow,
        const QModelIndex &sourceParent) const
{
  if (!filter_source_.empty()) {
    QModelIndex index = sourceModel()->index(sourceRow,
                                             StreamModel::COLUMN_SOURCE,
                                             sourceParent);
    char src = sourceModel()->data(index, StreamModel::FilterRole)
               .toChar().toLatin1();

    foreach (char c, filter_source_) {
      if (src == c) return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// LogStream

LogView::LogView(TinaInterface *interface, QWidget *parent) :
    BaseFrontend("Meldungen", parent), scroll_on_output_(false)
{
  icons[StreamModel::ICON_WARNING]  = QIcon(":/images/warning-orange-16.png");
  icons[StreamModel::ICON_CRITICAL] = QIcon(":/images/error-orange-16.png");
  icons[StreamModel::ICON_ERROR]    = QIcon(":/images/error-red-16.png");

  model_ = new StreamModel(this);

  filter_ = new LogFilter(this);
  filter_->setSourceModel(model_);
//  filter_mapper_ = new QSignalMapper(this);
//  connect(filter_mapper_, SIGNAL(mapped(int)), this, SLOT(onFilterSource(int)));

  table_ = new QTableView;
  table_->setModel(filter_);
  table_->setSelectionBehavior(QAbstractItemView::SelectRows);
  table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  table_->setWordWrap(true);
  table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_->setFont(QFont("Consolas", 9));
  table_->setShowGrid(false);
  table_->setContextMenuPolicy(Qt::CustomContextMenu);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(table_);
  setLayout(layout);

  QHeaderView* hheader = table_->horizontalHeader();
  hheader->hide();
#if QT_VERSION < 0x050000
  hheader->setResizeMode(StreamModel::COLUMN_TIME, QHeaderView::Fixed);
  hheader->setResizeMode(StreamModel::COLUMN_MESSAGE, QHeaderView::Stretch);
  hheader->setResizeMode(StreamModel::COLUMN_SOURCE, QHeaderView::ResizeToContents);
  hheader->setResizeMode(StreamModel::COLUMN_ICON, QHeaderView::Fixed);
#else
  hheader->setSectionResizeMode(StreamModel::COLUMN_TIME, QHeaderView::Fixed);
  hheader->setSectionResizeMode(StreamModel::COLUMN_MESSAGE, QHeaderView::Stretch);
  hheader->setSectionResizeMode(StreamModel::COLUMN_SOURCE, QHeaderView::ResizeToContents);
  hheader->setSectionResizeMode(StreamModel::COLUMN_ICON, QHeaderView::Fixed);
#endif
  hheader->resizeSection(StreamModel::COLUMN_ICON, 20);
  hheader->resizeSection(StreamModel::COLUMN_TIME, 50);

  QHeaderView* vheader = table_->verticalHeader();
  vheader->hide();
  vheader->setDefaultSectionSize(22);

  connect(table_, SIGNAL(activated(QModelIndex)),
          this, SLOT(activated(QModelIndex)));
  connect(table_, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(contextMenu(QPoint)));

  connect(interface, SIGNAL(beginUpdate()), this, SLOT(beginUpdate()));
  connect(interface, SIGNAL(endUpdate()), this, SLOT(endUpdate()));
  connect(interface, SIGNAL(tinaPackageReady(QByteArray)), this, SLOT(writeLine(QByteArray)));
}

static
const char* charToKey(char c) {
  static char key[2];
  key[0] = c;
  key[1] = '\0';
  return key;
}

void LogView::onConnected(bool readOnly, bool isSequential) {
    (void)isSequential;

    setScrollOnOutput(!readOnly);

  if (!readOnly) {
      emit dataReady(QByteArray(">"));
  }
}

void LogView::onDisconnected(bool reconnecting) {
    (void) reconnecting;

    QSettings settings;
    const std::string& filter = filter_->getFilterSource();
    for (char key : filter) {
      if (std::find(filter.begin(), filter.end(), key) != filter.end()) {
        settings.setValue(charToKey(key), false);
      } else {
        settings.remove(charToKey(key));
      }
    }
}

void LogView::beginUpdate() {
  model_->beginUpdate();
}

void LogView::endUpdate() {
  model_->endUpdate();
}

void LogView::setScrollOnOutput(bool on) {
  if (scroll_on_output_ != on) {
    scroll_on_output_ = on;
    if (on) {
      connect(table_->verticalHeader(), SIGNAL(sectionCountChanged(int,int)),
              this, SLOT(scroll(int,int)));
    } else {
      disconnect(this, SLOT(scroll(int,int)));
    }
  }
}

void LogView::scroll(int, int) {
  QScrollBar* scrollbar = table_->verticalScrollBar();

  if (scrollbar->value() == scrollbar->maximum()) {
    table_->scrollToBottom();
  }
}

void LogView::activated(QModelIndex index) {
  table_->resizeRowToContents(index.row());
}

void LogView::contextMenu(QPoint point) {
  QMenu menu;

  // Makierung
  QItemSelectionModel* selection = table_->selectionModel();
  if (selection && selection->hasSelection()) {
    menu.addAction("&Kopieren", this, SLOT(copy()));
    menu.addAction("&Nachrichten von Quelle ausblenden", this, SLOT(hideMsgsFromSource()));
  }
  menu.addAction("&Alles markieren", table_, SLOT(selectAll()));
  menu.addSeparator();

  // Filter
  filter_mapper_ = new QSignalMapper(this);
  connect(filter_mapper_, SIGNAL(mapped(int)), this, SLOT(onFilterSrc(int)));

  QMenu* filter_menu = new QMenu("Filter einstellen");
  const QString* sources = model_->getLogSources();
  std::string filter = filter_->getFilterSource();
  for (int i = 33; i < 127; i++) {
    if (sources[i].length() > 0) {
      QAction *action  = filter_menu->addAction(sources[i]);
      action->setCheckable(true);
      action->setChecked(std::find(filter.cbegin(), filter.cend(), i) == filter.cend());
      connect(action, SIGNAL(triggered()), filter_mapper_, SLOT(map()));
//      connect(action, SIGNAL(triggered()), this, SLOT(hideMsgsFromSource()));
      filter_mapper_->setMapping(action, i);
    }
  }
  menu.addMenu(filter_menu);

  menu.exec(table_->mapToGlobal(point));
}

void LogView::copy() {
  QItemSelectionModel* selection = table_->selectionModel();
  if (selection && selection->hasSelection()) {
    QString result;
    QModelIndexList sel = selection->selectedRows(StreamModel::COLUMN_MESSAGE);
    for (auto iter = sel.begin(); iter != sel.end(); iter++) {
      result.append(iter->data().toString());
      result.append('\n');
    }

    QApplication::clipboard()->setText(result);
  }
}

void LogView::hideMsgsFromSource() {
  QItemSelectionModel* selection = table_->selectionModel();
  if (selection && selection->hasSelection()) {
    std::string filter = filter_->getFilterSource();
    QModelIndexList sel = selection->selectedRows(StreamModel::COLUMN_SOURCE);
    for (auto iter = sel.begin(); iter != sel.end(); iter++) {
      char c = static_cast<char>(iter->data(StreamModel::FilterRole).toUInt());
      if (std::find(filter.cbegin(), filter.cend(), c) == filter.cend()) {
        filter.push_back(c);
      }
    }
    filter_->setFilterSource(std::move(filter));
  }
}

void LogView::onFilterSrc(int index) {
  std::string filter = filter_->getFilterSource();
  std::string::iterator i = std::find(filter.begin(), filter.end(), index);
  if (i == filter.end()) {
    // nicht im Filter vorhanden -> aktiv
    filter.push_back(index);

  } else {
    // im Filter vorhanden -> inaktiv
    filter.erase(i);
  }
  filter_->setFilterSource(std::move(filter));
}

void LogView::setLogSource(char source, const QString&& name) {
  QSettings settings;
  if (!settings.value(charToKey(source), true).toBool()) {
    std::string filter = filter_->getFilterSource();
    filter.push_back(source);
    filter_->setFilterSource(std::move(filter));
  }

  model_->setLogSource(source, std::move(name));
}

void LogView::writeLine(QByteArray line) {
	if (line.size() > 2) {
		char level = line.at(1);
		unsigned source = line.at(0);
		line.remove(0, 2);

		switch (level) {
		case '-':
		case '!':
		case '?':
			insertRow(level, line.data(), line.size(), source);
			break;

		case '>':
			if (std::isprint(source)) {
				setLogSource(source, QString::fromUtf8(line));
			}
			break;

		case ';':
			if (source == ';') {
				insertRow(level, line.data(), line.size(), source);
			}
			break;
		}
	}
}


void LogView::clear(void) {
    model_->clear();
}

bool LogView::saveOutput(void) {
    return false;
}

void LogView::writeData(QByteArray data)
{
}
