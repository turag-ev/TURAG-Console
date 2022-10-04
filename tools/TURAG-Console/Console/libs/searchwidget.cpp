#include "searchwidget.h"

#include <QSortFilterProxyModel>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QAction>
#include <QLineEdit>
#include <QBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>


// SearchModel

SearchModel::SearchModel(QObject* parent)
    : QObject(parent)
{ }

void SearchModel::setSearchTask(const SearchTask &search)
{
    task_ = search;
    doSearch();
}

void SearchModel::setViewSettings(const SearchViewSettings &search) {
	view_settings_ = search;
}

// TableViewSearchModel

TableViewSearchModel::TableViewSearchModel(QObject *parent)
    : SearchModel(parent)
{
	model_ = new QSortFilterProxyModel(this);
	model_->setDynamicSortFilter(false);

    // search model
    connect(model_, &QSortFilterProxyModel::rowsInserted,
    [=](const QModelIndex& index, int start, int end)
    {
        //qDebug() << "TableViewSearchModel::rowsInserted" << start << end;

        for (int i = start; i <= end; i++)
        {
            model_->setData(model_->index(i, 0), true, hightlight_role_);
        }
        emit resultCountChanged(model_->rowCount());
    });
    connect(model_, &QSortFilterProxyModel::rowsAboutToBeRemoved,
    [=](const QModelIndex& index, int start, int end)
    {
        //qDebug() << "TableViewSearchModel::rowsAboutToBeRemoved";

		// FIXME: cur_search_index_ anpassen
        for (int i = start; i <= end; i++)
        {
            model_->setData(model_->index(i, 0), false, hightlight_role_);
        }
    });
	connect(model_, &QSortFilterProxyModel::rowsRemoved,
	[=](const QModelIndex& index, int start, int end)
	{
        //qDebug() << "TableViewSearchModel::rowsRemoved" << start << end;
        emit resultCountChanged(model_->rowCount());
	});
}

void TableViewSearchModel::setTableView(QTableView *view)
{
    endSearch();

    view_ = view;
    if (view_)
    {
		setSourceModel(view->model());
		doSearch();
    }
    else
    {
		setSourceModel(nullptr);
    }
}

int TableViewSearchModel::searchKeyColumn() const
{
    return model_->filterKeyColumn();
}

void TableViewSearchModel::setSearchKeyColumn(int value)
{
    model_->setFilterKeyColumn(value);
}

void TableViewSearchModel::doSearch()
{
	endSearch();

    QString text = searchTask().text();
    if (!text.isEmpty())
    {
		setSourceModel(view_->model());

		// set search text
		switch (searchTask().type())
		{
		case SearchType::FixedString:
			model_->setFilterFixedString(text);
			break;

		case SearchType::Wildcard:
			model_->setFilterWildcard(text);
			break;

		case SearchType::RegEx:
			model_->setFilterRegExp(text);
			break;
		}

		// set case sensitivity
		Qt::CaseSensitivity cs = searchTask().caseSensitive()
				? Qt::CaseSensitive : Qt::CaseInsensitive;
		if (cs != model_->filterCaseSensitivity()) {
			model_->setFilterCaseSensitivity(cs);
		}

		// search and highlight
        if (setSearchResult(model_->index(model_->rowCount()-1,0))) {
            // found one
            for (int i = 0; i < model_->rowCount(); i++)
            {
                model_->setData(model_->index(i, 0), true, hightlight_role_);
            }

            return;
        }
    }
}

int TableViewSearchModel::resultCount() const
{
    return model_->rowCount();
}

int TableViewSearchModel::currentResult() const
{
	return cur_index_ + 1;
}


void TableViewSearchModel::updateSearchAfterSourceReset()
{
    doSearch();
    emit resultCountChanged(model_->rowCount());
}

void TableViewSearchModel::showNextResult()
{
    if (model_->rowCount() == 0) return;

    int new_row = cur_index_ + 1;
    if (new_row >= model_->rowCount())
    {
        // TODO: make animation
        new_row = 0;
    }

    setSearchResult(model_->index(new_row, 0));
}

void TableViewSearchModel::showPreciousResult()
{
    if (model_->rowCount() == 0) return;

    int new_row = cur_index_ - 1;
    if (new_row < 0)
    {
        // TODO: make animation
        new_row = model_->rowCount() - 1;
    }

    setSearchResult(model_->index(new_row, 0));
}

void TableViewSearchModel::endSearch()
{
    if (!view_ || !model_->sourceModel()) return;

    view_->clearSelection();
    cur_index_ = -1;

    for (int i = 0; i < model_->rowCount(); i++)
    {
        model_->setData(model_->index(i, 0), false, hightlight_role_);
    }

	setSourceModel(nullptr);
}

bool TableViewSearchModel::setSearchResult(QModelIndex search_index)
{
    auto index = model_->mapToSource(search_index);

    if (!index.isValid()) return false;

    cur_index_ = search_index.row();
    view_->scrollTo(index, QAbstractItemView::EnsureVisible);
	view_->clearSelection();
    view_->setCurrentIndex(index);

	return true;
}

void TableViewSearchModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	if (model_->sourceModel()) {
        disconnect(model_->sourceModel(), SIGNAL(modelReset()), this, SLOT(updateSearchAfterSourceReset()));
	}
	if (sourceModel) {
        connect(sourceModel, SIGNAL(modelReset()), this, SLOT(updateSearchAfterSourceReset()), Qt::QueuedConnection);
	}
	model_->setSourceModel(sourceModel);
}

// SearchWidget

namespace {

class CloseSearchByEsc : public QObject {
public:
  CloseSearchByEsc(SearchWidget* obj) :
    QObject(obj)
  { }

  bool eventFilter(QObject *obj, QEvent *event) override
  {
      if (event->type() == QEvent::KeyPress
		  && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape
         )
      {
          static_cast<SearchWidget*>(parent())->hide();
		  return true;
      }

	  return false;
  }
};

}

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent)
{
    // search field
    search_field_ = new QLineEdit();
    search_field_->setPlaceholderText(QStringLiteral("Suche"));
    search_field_->setClearButtonEnabled(true);

    connect(search_field_, SIGNAL(textChanged(QString)), this, SLOT(search()));
    connect(search_field_, SIGNAL(returnPressed()), this, SLOT(goDown()));

    // actions
    start_search_ = new QAction(
                QIcon::fromTheme("system-search"),
                QStringLiteral("Suche ..."),
                this);
    start_search_->setShortcut(QKeySequence(QKeySequence::Find));
	start_search_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(start_search_, SIGNAL(triggered(bool)), this, SLOT(openSearch()));

    search_down_ = new QAction(
                QIcon::fromTheme("go-down", QIcon(":/libsimeurobot/img/arrow-down-16.png")),
                QStringLiteral("Weitersuchen"),
                this);
    search_down_->setShortcut(QKeySequence(QKeySequence::FindPrevious));
	search_down_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(search_down_, SIGNAL(triggered(bool)), this, SLOT(goDown()));

    search_up_ = new QAction(
                QIcon::fromTheme("go-up", QIcon(":/libsimeurobot/img/arrow-up-16.png")),
                QStringLiteral("Rückwärts suchen"),
                this);
    search_up_->setShortcut(QKeySequence(QKeySequence::FindNext));
	search_up_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(search_up_, SIGNAL(triggered(bool)), this, SLOT(goUp()));

    search_field_->addAction(search_down_, QLineEdit::ActionPosition::LeadingPosition);
    search_field_->addAction(search_up_, QLineEdit::ActionPosition::LeadingPosition);

    // result
    search_result_ = new QLabel();
    search_result_->setMargin(2);
    search_result_->setIndent(5);

    // close
    auto close_button = new QPushButton();
    close_button->setIcon(QIcon(":/libsimeurobot/img/cross-16.png"));
    close_button->setFlat(true);
    connect(close_button, SIGNAL(clicked(bool)), this, SLOT(hide()));

    // build layout
    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(3);
    layout->addWidget(search_field_);
    layout->addWidget(search_result_);
    layout->addWidget(close_button);

    // failed search
    orig_search_field_ = search_field_->palette();
    failed_search_field_.setColor(QPalette::Text, Qt::red);

    // close search
    search_field_->installEventFilter(new CloseSearchByEsc(this));

    // set states
    resultsChanged();
}

void SearchWidget::setHidable(bool value)
{
    hidable_ = value;
}

void SearchWidget::setModel(SearchModel *value)
{
    if (model_ == value) return;

    if (model_)
        disconnect(model_, SIGNAL(resultCountChanged(int)), this, SLOT(resultsChanged()));

    model_ = value;

    if (model_)
        connect(model_, SIGNAL(resultCountChanged(int)), this, SLOT(resultsChanged()));
}

void SearchWidget::goUp()
{
    if (!model_) return;

    if (!searching_)
    {
        search();
        return;
    }

    model_->gotoPreciousResult();
    resultsChanged();
}

void SearchWidget::goDown()
{
    if (!model_) return;

    if (!searching_)
    {
        search();
        return;
    }

    model_->gotoNextResult();
    resultsChanged();
}

void SearchWidget::openSearch()
{
    setVisible(true);
    search_field_->setFocus();
}

void SearchWidget::search()
{
    if (!model_) return;

    searching_ = true;

    SearchTask task = model_->searchTask();
    task.setText(search_field_->text());
    model_->setSearchTask(task);

    resultsChanged();
}

void SearchWidget::resultsChanged()
{
    int count = model_ ? model_->resultCount() : 0;
    int cur = model_ ? model_->currentResult() : 0;

    search_result_->setText(
        QString("%1 / %2").arg(cur).arg(count)
    );

    if (count == 0 && model_ && !model_->searchTask().isEmpty())
    {
        search_field_->setPalette(failed_search_field_);
    }
    else
    {
        search_field_->setPalette(orig_search_field_);
    }

    search_up_->setEnabled(count != 0);
    search_down_->setEnabled(count != 0);
}

void SearchWidget::hideEvent(QHideEvent *event)
{
    if (isVisible()) return;

    model_->clearSearch();
    resultsChanged();
    searching_ = false;
}

void SearchWidget::showEvent(QShowEvent *event)
{
    if (isVisible())
    {
         search_field_->setPalette(orig_search_field_);
    }
}


