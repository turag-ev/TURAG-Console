#define TURAG_DEBUG_LOG_SOURCE "_"

#include <tina/debug/print.h>
#include <tina++/debug/graph.h>

#include "logview.h"

#include <QTableView>
#include <QSettings>
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
#include <iterator>
#include <QVBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <tina++/algorithm.h>
#include <QList>
#include <QDebug>
#include <QKeyEvent>

#include "util/tinainterface.h"

static QIcon icons[StreamModel::ICON_MAX];

////////////////////////////////////////////////////////////////////////////////
// StreamModel

StreamModel::StreamModel(QObject* parent) :
    QAbstractTableModel(parent),
    row_buffer_(),
    rows_(),
    last_size(),
    log_sources_(),
    logtime_(0)
{
	log_sources_[';'] = QStringLiteral("System");
    connect(&insertTimer, SIGNAL(timeout()), this, SLOT(insertRowsTimeout()));
    insertTimer.setInterval(100);
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

    int column = index.column();
    int row = index.row();

    if (row >= static_cast<int>(rows_.size()) || row < 0)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:{
		const Row& data = rows_[row];

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
			// TODO: PERF: QString.arg ersetzen
			return QStringLiteral("%1").arg(std::get<DATA_TIME>(data), 0, 'f', 3);
        }
        break;
    }

    case Qt::DecorationRole:
        if (column == COLUMN_ICON) {
            return icons[std::get<DATA_ICON>(rows_[row])];
        }
        break;

    case FilterRole:
        if (column == COLUMN_SOURCE) {
            return std::get<DATA_SOURCE>(rows_[row]);
        }
        break;

    case Qt::BackgroundRole:
        if (std::get<DATA_SOURCE>(rows_[row]) == ';') {
            return QColor(255, 128, 0);
        }
        break;

    case Qt::ForegroundRole:
        if (std::get<DATA_SOURCE>(rows_[row]) == ';') {
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
				return QStringLiteral("Nachricht");

            case COLUMN_SOURCE:
				return QStringLiteral("Quelle");

            case COLUMN_ICON:
				return QString();

            default:
                return QVariant();
            }
        }
    }

    return QVariant();
}

bool StreamModel::insertRow(char level, const char *data, std::size_t len, unsigned source)
{
    // icon
    Icon icon;
    switch (level) {
    case TURAG_DEBUG_INFO_PREFIX[0]:        icon = ICON_INFO;     break;
    case TURAG_DEBUG_WARN_PREFIX[0]:        icon = ICON_WARNING;  break;
    case TURAG_DEBUG_CRITICAL_PREFIX[0]:    icon = ICON_CRITICAL;  break;
    case TURAG_DEBUG_ERROR_PREFIX[0]:       icon = ICON_ERROR;    break;
    default:                                icon = ICON_INFO;     break;
    }

    if (level == TURAG_DEBUG_GAMETIME_PREFIX[0]) {
        int hex_time;

        if (sscanf(data, "%x", &hex_time) == 1) {
            logtime_ = static_cast<float>(hex_time) / 1000.0f;
            return true;
        } else {
            return false;
        }
    }

    if (log_sources_[source].isNull()) {
		log_sources_[source] = QString();
    }

    row_buffer_.emplace_back(icon, QString::fromUtf8(data, len), source, logtime_);

    if (!insertTimer.isActive()) {
        insertTimer.start();
    }

    return true;
}


void StreamModel::insertRowsTimeout(void) {
    if (row_buffer_.size()) {
        beginInsertRows(QModelIndex(), rows_.size(), rows_.size() + row_buffer_.size() - 1);
        std::move(row_buffer_.begin(), row_buffer_.end(), std::back_inserter(rows_));
        endInsertRows();
        row_buffer_.clear();
        insertTimer.stop();
    }
}

void StreamModel::clear() {
    int size = rows_.size();
    if (size) {
        beginRemoveRows(QModelIndex(), 0, size);

        rows_.clear();

        endRemoveRows();
    }
    logtime_ = 0;
    row_buffer_.clear();
    insertTimer.stop();

    for (unsigned i = 0; i < sizeof(log_sources_) / sizeof(log_sources_[0]); ++i) {
        log_sources_[i] = QString();
    }
	log_sources_[';'] = QStringLiteral("System");
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
                   .toUInt();

        return !TURAG::any_of_value(filter_source_, src);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// LogView

LogView::LogView(TinaInterface *interface, QWidget *parent) :
	BaseFrontend(QStringLiteral("Meldungen"), parent), scroll_on_output_(false), hasReadOnlyConnection(false)
{
    // icons
	icons[StreamModel::ICON_WARNING]  = QIcon(QStringLiteral(":/images/warning-orange-16.png"));
	icons[StreamModel::ICON_CRITICAL] = QIcon(QStringLiteral(":/images/error-orange-16.png"));
	icons[StreamModel::ICON_ERROR]    = QIcon(QStringLiteral(":/images/error-red-16.png"));

    // log
    log_model_ = new StreamModel(this);

    filter_ = new LogFilter(this);
    filter_->setSourceModel(log_model_);
    //  filter_mapper_ = new QSignalMapper(this);
    //  connect(filter_mapper_, SIGNAL(mapped(int)), this, SLOT(onFilterSource(int)));

    log_ = new QTableView;
    log_->setModel(filter_);
    log_->setSelectionBehavior(QAbstractItemView::SelectRows);
    log_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    log_->setWordWrap(true);
    log_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QFont font;
    font.setStyleHint(QFont::System);
    font.setPointSize(8);
    log_->setFont(font);
    log_->setShowGrid(false);
    log_->setContextMenuPolicy(Qt::CustomContextMenu);
    log_->setFocusPolicy(Qt::NoFocus);

    QHeaderView* hheader = log_->horizontalHeader();
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

    QHeaderView* vheader = log_->verticalHeader();
    vheader->hide();
    vheader->setDefaultSectionSize(22);

    connect(log_, SIGNAL(activated(QModelIndex)),
            this, SLOT(activated(QModelIndex)));
    connect(log_, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenu(QPoint)));

    // build gui
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(log_);
    layout->setMargin(0);
    setLayout(layout);

    // connect to input source TinaInterface
    connect(interface, SIGNAL(tinaPackageReady(QByteArray)), this, SLOT(writeLine(QByteArray)));

    connect(&sendTimer, SIGNAL(timeout()), this, SLOT(onSendTimeout()));

    readSettings();
}

LogView::~LogView() {
    writeSettings();
}

void LogView::onSendTimeout(void) {
    if (timedSendString.size()) {
        emit dataReady(timedSendString.left(1));
        timedSendString.remove(0,1);
    }
    if (timedSendString.size() == 0) {
        sendTimer.stop();
    }
}

static
const char* charToKey(char c) {
    static char key[2];
    key[0] = c;
    key[1] = '\0';
    return key;
}

void LogView::onConnected(bool readOnly, bool isBuffered, QIODevice* dev) {
    (void)isBuffered;
    (void)dev;

    setScrollOnOutput(!readOnly);

    if (!readOnly) {
        timedSendString = "\x18\x18\x18\x18\x18\x18\x18>\r\n";
        sendTimer.start(10);
    }

    hasReadOnlyConnection = readOnly;
//    log_->setEnabled(true);
}

void LogView::onDisconnected(bool reconnecting) {
    (void) reconnecting;

    QSettings settings;
    const std::string& filter = filter_->getFilterSource();
    for (char key : filter) {
        if (TURAG::any_of_value(filter, key)) {
            settings.setValue(charToKey(key), false);
        } else {
            settings.remove(charToKey(key));
        }
    }

//    log_->setEnabled(false);
}


void LogView::setScrollOnOutput(bool on) {
    if (scroll_on_output_ != on) {
        scroll_on_output_ = on;
        if (on) {
            connect(log_->verticalHeader(), SIGNAL(sectionCountChanged(int,int)),
                    this, SLOT(scroll(int,int)));
        } else {
            disconnect(this, SLOT(scroll(int,int)));
        }
    }
}

void LogView::scroll(int, int) {
    QScrollBar* scrollbar = log_->verticalScrollBar();

    if (scrollbar->value() == scrollbar->maximum()) {
        log_->scrollToBottom();
    }
}

void LogView::activated(QModelIndex index) {
    log_->resizeRowToContents(index.row());
    log_->resizeColumnsToContents();

	if (index.column() != StreamModel::COLUMN_MESSAGE) {
		index = index.sibling(index.row(), StreamModel::COLUMN_MESSAGE);
	}

	if (index.isValid()) {
		QString line = index.data().toString();
		if (line.startsWith(QStringLiteral("Graph")) && line.size() > 6) {
			bool ok = false;
			int index = line.mid(6, line.indexOf(':') - 6).toInt(&ok);

			if (ok) {
				emit activatedGraph(index);
			}
		}
	}
}

void LogView::contextMenu(QPoint point) {
    QMenu menu;

    // Makierung
    QItemSelectionModel* selection = log_->selectionModel();
    if (selection && selection->hasSelection()) {
		menu.addAction(QStringLiteral("&Kopieren"), this, SLOT(copy()));
		menu.addAction(QStringLiteral("&Nachrichten von Quelle ausblenden"), this, SLOT(hideMsgsFromSource()));
    }
	menu.addAction(QStringLiteral("&Alles markieren"), log_, SLOT(selectAll()));
	if (!hasReadOnlyConnection) menu.addAction(QStringLiteral("Ausgabe löschen"), this, SLOT(clear()));
    menu.addSeparator();

    // Filter
    filter_mapper_ = new QSignalMapper(this);
    connect(filter_mapper_, SIGNAL(mapped(int)), this, SLOT(filterSrc(int)));

	QMenu* filter_menu = new QMenu(QStringLiteral("Filter einstellen"));
	auto sources = log_model_->getLogSources();
    std::string filter = filter_->getFilterSource();
    for (int i = 33; i < 127; i++) {
        if (!sources[i].isNull()) {
            QAction *action  = filter_menu->addAction(sources[i].isEmpty()
                                                      ? QString(static_cast<char>(i))
                                                      : sources[i]);
            action->setCheckable(true);
            action->setChecked(!TURAG::any_of_value(filter, i));
            connect(action, SIGNAL(triggered()), filter_mapper_, SLOT(map()));
            //      connect(action, SIGNAL(triggered()), this, SLOT(hideMsgsFromSource()));
            filter_mapper_->setMapping(action, i);
        }
    }
    menu.addMenu(filter_menu);
	menu.addAction(QStringLiteral("&alles anzeigen"), this, SLOT(deactivateFilter()));
	menu.addAction(QStringLiteral("&nichts anzeigen"), this, SLOT(activateFilter()));

    menu.exec(log_->mapToGlobal(point));
}

void LogView::copy() {
    QItemSelectionModel* selection = log_->selectionModel();
    if (selection && selection->hasSelection()) {
        QString result;
        for (const auto& row : selection->selectedRows(StreamModel::COLUMN_MESSAGE)) {
            result.append(row.data().toString());
            result.append('\n');
        }

        QApplication::clipboard()->setText(result);
    }
}

void LogView::hideMsgsFromSource() {
    QItemSelectionModel* selection = log_->selectionModel();
    if (selection && selection->hasSelection()) {

        std::string filter = filter_->getFilterSource();
        for (const auto& row : selection->selectedRows(StreamModel::COLUMN_SOURCE)) {
            char c = static_cast<char>(row.data(StreamModel::FilterRole).toUInt());
            if (!TURAG::any_of_value(filter, c)) {
                filter.push_back(c);
            }
        }

        filter_->setFilterSource(std::move(filter));
    }
}

void LogView::filterSrc(int index) {
    std::string filter = filter_->getFilterSource();
    auto i = std::find(filter.begin(), filter.end(), index);
    if (i == filter.end()) {
        // nicht im Filter vorhanden -> aktiv
        filter.push_back(index);

    } else {
        // im Filter vorhanden -> inaktiv
        TURAG::remove(filter, index); // sicher ist sicher, wenn mehrfach vorhanden
    }
    filter_->setFilterSource(std::move(filter));
}

void LogView::deactivateFilter() {
    filter_->setFilterSource(std::string());
}

void LogView::activateFilter() {
    std::string filter;
    filter.reserve(20);

	auto sources = log_model_->getLogSources();
    for (int i = 33; i < 127; i++) {
        if (!sources[i].isNull()) {
            filter.push_back(i);
        }
    }
    filter_->setFilterSource(std::move(filter));
}

void LogView::readSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
	filter_->setFilterSource(settings.value(QStringLiteral("filter"), QString()).toString().toStdString());
}

void LogView::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
	settings.setValue(QStringLiteral("filter"), QString::fromStdString(filter_->getFilterSource()));
}

void LogView::writeLine(QByteArray line) {
    if (line.size() > 2) {
        char level = line.at(1);
        unsigned source = line.at(0);
        line.remove(0, 2);

        switch (level) {
        case TURAG_DEBUG_ERROR_PREFIX[0]:
        case TURAG_DEBUG_CRITICAL_PREFIX[0]:
        case TURAG_DEBUG_WARN_PREFIX[0]:
        case TURAG_DEBUG_INFO_PREFIX[0]:
        case TURAG_DEBUG_DEBUG_PREFIX[0]:
        case TURAG_DEBUG_GAMETIME_PREFIX[0]:
            insertRow(level, line.data(), line.size(), source);
            break;

		case TURAG_DEBUG_GRAPH_PREFIX[0]:
			if (source == TURAG_DEBUG_GRAPH_CREATE[0]) {
                QTextStream stream(line);
                int index = 0;
                stream >> index;
				QString graphline = QStringLiteral("Graph %1: '%2'").arg(index).arg(stream.readAll().trimmed());
                insertRow(';', graphline.toLatin1().constData(), graphline.size(), ';');
			} else if (source == TURAG_DEBUG_GRAPH_COPY[0]) {
				QTextStream stream(line);
				int index = 0, old_index;
				stream >> index;
				stream >> old_index;
				QString graphline = QStringLiteral("Graph %1: '%2'").arg(index).arg(stream.readAll().trimmed());
				insertRow(';', graphline.toLatin1().constData(), graphline.size(), ';');
			}
            break;

        case TURAG_DEBUG_REPORT_LOG_SOURCE_PREFIX[0]:
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
    log_model_->clear();
}

// we need to implement this function as part of the base classes interface
// but we don't actually need it.
void LogView::writeData(QByteArray data)
{
    Q_UNUSED(data);
}

void LogView::keyPressEvent ( QKeyEvent * e ) {
  if (e->count() > 0) {
      qDebug() << "PTF keyPressEvent: '" << e->text().toUtf8() << "'";
      emit dataReady(e->text().toUtf8());
  } else {
      BaseFrontend::keyPressEvent(e);
  }
}
