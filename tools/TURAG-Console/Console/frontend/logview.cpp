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
#include <QVBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <tina++/algorithm.h>
#include <QList>
#include <QDebug>

#include "util/tinainterface.h"

static QIcon icons[StreamModel::ICON_MAX];

////////////////////////////////////////////////////////////////////////////////
// StreamModel

StreamModel::StreamModel(QObject* parent) :
    QAbstractTableModel(parent),
    rows_(),
    last_size(),
    log_sources_(),
    logtime_(0)
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

    int column = index.column();
    int row = index.row();

    if (row >= static_cast<int>(rows_.size()) || row < 0)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:{
        Row data = rows_[row];

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

    if (source == ';' && len > 10 &&
        strncmp(data, "SPIELZEIT:", 10) == 0)
    {
        QTextStream stream(QByteArray(data, len));
        stream.seek(10); // "SPIELZEIT:"

        int sec = 0, msec = 0;
        stream >> sec;
        if (!stream.atEnd()) {
            stream.seek(stream.pos() + 1); // ,
            stream >> msec;
        }
        logtime_ = sec + msec / 1000.f;
        return true;
    }

    if (log_sources_[source].isNull()) {
        log_sources_[source] = "";
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
    if (size) {
        beginRemoveRows(QModelIndex(), 0, size);

        rows_.clear();

        endRemoveRows();
    }
    logtime_ = 0;
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
    BaseFrontend("Meldungen", parent), scroll_on_output_(false)
{
    // icons
    icons[StreamModel::ICON_WARNING]  = QIcon(":/images/warning-orange-16.png");
    icons[StreamModel::ICON_CRITICAL] = QIcon(":/images/error-orange-16.png");
    icons[StreamModel::ICON_ERROR]    = QIcon(":/images/error-red-16.png");

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
    log_->setFont(QFont("Consolas", 9));
    log_->setShowGrid(false);
    log_->setContextMenuPolicy(Qt::CustomContextMenu);

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
    connect(interface, SIGNAL(beginUpdate()), this, SLOT(beginUpdate()));
    connect(interface, SIGNAL(endUpdate()), this, SLOT(endUpdate()));
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

void LogView::onConnected(bool readOnly, bool isSequential, QIODevice* dev) {
    (void)isSequential;
    (void)dev;

    setScrollOnOutput(!readOnly);

    if (!readOnly) {
        timedSendString = "\x18\x18\x18\x18\x18\x18\x18>";
        sendTimer.start(10);
    }

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

void LogView::beginUpdate() {
    log_model_->beginUpdate();
}

void LogView::endUpdate() {
    log_model_->endUpdate();
    log_->resizeRowsToContents();
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

    StreamModel::Row data = log_model_->rows()[index.row()];
    QString line = std::get<StreamModel::DataColumn::DATA_MESSAGE>(data);
    if (line.startsWith("Graph") && line.size() > 6) {
        bool ok = false;
        int index = line.mid(6, line.indexOf(':') - 6).toInt(&ok);

        if (ok) {
            emit activatedGraph(index);
            qDebug() << "emit activatedGraph" << index;
        }
    }
}

void LogView::contextMenu(QPoint point) {
    QMenu menu;

    // Makierung
    QItemSelectionModel* selection = log_->selectionModel();
    if (selection && selection->hasSelection()) {
        menu.addAction("&Kopieren", this, SLOT(copy()));
        menu.addAction("&Nachrichten von Quelle ausblenden", this, SLOT(hideMsgsFromSource()));
    }
    menu.addAction("&Alles markieren", log_, SLOT(selectAll()));
    menu.addSeparator();

    // Filter
    filter_mapper_ = new QSignalMapper(this);
    connect(filter_mapper_, SIGNAL(mapped(int)), this, SLOT(filterSrc(int)));

    QMenu* filter_menu = new QMenu("Filter einstellen");
    const QString* sources = log_model_->getLogSources();
    std::string filter = filter_->getFilterSource();
    for (int i = 33; i < 127; i++) {
        if (!sources[i].isNull()) {
            QAction *action  = filter_menu->addAction(sources[i].isEmpty()
                                                      ? QString(static_cast<char>(i))
                                                      : sources[i]);
            action->setCheckable(true);
            action->setChecked(TURAG::any_of_value(filter, i));
            connect(action, SIGNAL(triggered()), filter_mapper_, SLOT(map()));
            //      connect(action, SIGNAL(triggered()), this, SLOT(hideMsgsFromSource()));
            filter_mapper_->setMapping(action, i);
        }
    }
    menu.addMenu(filter_menu);
    menu.addAction("&nichts filtern", this, SLOT(deactivateFilter()));
    menu.addAction("&alles filtern", this, SLOT(activateFilter()));

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

    const QString* sources = log_model_->getLogSources();
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
    filter_->setFilterSource(settings.value("filter", QString()).toString().toStdString());
}

void LogView::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("filter", QString::fromStdString(filter_->getFilterSource()));
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

        case 'D':
            if (source == 'n') {
                QTextStream stream(line);
                int index = 0;
                stream >> index;
                QString graphline = QString("Graph %1: '%2'").arg(index).arg(stream.readAll().trimmed());
                insertRow(';', graphline.toLatin1().constData(), graphline.size(), ';');
            }
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
    log_model_->clear();
}

bool LogView::saveOutput(void) {
    QString filename = QFileDialog::getSaveFileName(this);

    if (filename.isEmpty()) {
        return true;
    }

    QFile savefile(std::move(filename));
    if (!savefile.open(QIODevice::WriteOnly)) {
        return false;
    }

    if (!savefile.isWritable()) {
        return false;
    }

    for (const auto& row : log_model_->rows()) {
        char c = '\x02';
        savefile.write(&c, 1);

        c = static_cast<char>(std::get<StreamModel::DATA_SOURCE>(row));
        savefile.write(&c, 1);

        switch (std::get<StreamModel::DATA_ICON>(row)) {
        case StreamModel::ICON_INFO:     c = '-'; break;
        case StreamModel::ICON_WARNING:  c = '?'; break;
        case StreamModel::ICON_CRITICAL: c = '!'; break;
        case StreamModel::ICON_ERROR:    c = '!'; break;
        default:                         c = '!'; break;
        }
        savefile.write(&c, 1);

        savefile.write(std::get<StreamModel::DATA_MESSAGE>(row).toUtf8());

        c = '\n';
        savefile.write(&c, 1);
    }

    return true;
}

// we need to implement this function as part of the base classes interface
// but we don't actually need it.
void LogView::writeData(QByteArray data)
{
    Q_UNUSED(data);
}
