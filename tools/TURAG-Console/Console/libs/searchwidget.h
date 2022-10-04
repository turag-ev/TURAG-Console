#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QPalette>
#include <QModelIndex>

class QAction;
class QLineEdit;
class QLabel;
class QSortFilterProxyModel;
class QTableView;


enum class SearchType {
    FixedString,
	Wildcard,
    RegEx
};

class SearchTask
{
public:
    SearchTask()
    { }

    SearchTask(const QString& text)
        : text_(text)
    { }

    SearchType type() const { return type_; }
    void setType(SearchType type) { type_ = type; }

    bool caseSensitive() const { return case_sensitive_; }
    void setCaseSensitive(bool value) { case_sensitive_ = value; }

    QString text() const { return text_; }
    void setText(const QString& text) { text_ = text; }

    bool isEmpty() const { return text_.isEmpty(); }

private:
    SearchType type_ = SearchType::FixedString;
    bool case_sensitive_ = false;
    QString text_ = { };
};

class SearchViewSettings
{
public:
    SearchViewSettings()
    { }

    bool highlight() const { return highlight_; }
    void setHighlight(bool value) { highlight_ = value; }

private:
    bool highlight_ = true;
    // TODO: background-color
};

class SearchModel : public QObject
{
    Q_OBJECT
public:
    SearchModel(QObject* parent = nullptr);
    virtual ~SearchModel() = default;

    const SearchTask& searchTask() const { return task_; }
    virtual void setSearchTask(const SearchTask& search);
    void clearSearch() { setSearchTask(SearchTask()); }

    const SearchViewSettings& viewSettings() const { return view_settings_; }
    virtual void setViewSettings(const SearchViewSettings& search);

    void gotoNextResult() { showNextResult(); }
    void gotoPreciousResult() { showPreciousResult(); }

    virtual int resultCount() const = 0;
    virtual int currentResult() const = 0;

signals:
    void resultCountChanged(int);
    void taskChanged(const SearchTask&);
    void viewSettingsChanged(const SearchViewSettings&);

private:
    SearchTask task_;
    SearchViewSettings view_settings_;

    virtual void doSearch() = 0;
    virtual void showNextResult() = 0;
    virtual void showPreciousResult() = 0;
};

class TableViewSearchModel : public SearchModel
{
	Q_OBJECT
public:
    static const int HighlightRole = Qt::UserRole + 'S';

    explicit TableViewSearchModel(QObject *parent = 0);

    QTableView* tableView() const { return view_; }
    void setTableView(QTableView* view);

    void setHighlightRole(int role) { hightlight_role_ = role; }
    int highlightRole() const { return hightlight_role_; }

    int searchKeyColumn() const;
    void setSearchKeyColumn(int value);

    void doSearch() override;

    int resultCount() const override;
    int currentResult() const override;

private slots:
	void updateSearchAfterSourceReset();

private:
    QSortFilterProxyModel* model_;
    QTableView* view_;
    int cur_index_ = -1;
    int hightlight_role_ = HighlightRole;

    void showNextResult() override;
    void showPreciousResult() override;

    void endSearch();
    bool setSearchResult(QModelIndex search_index);

	void setSourceModel(QAbstractItemModel *sourceModel);
};

class SearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchWidget(QWidget *parent = 0);

    bool hidable() const { return hidable_; }
    void setHidable(bool value);

    SearchModel* model() const { return model_; }
    void setModel(SearchModel* value);

    QLineEdit* getSearchField() { return search_field_; }

    QAction* getOpenSearchAction() { return start_search_; }
    QAction* getSearchUpAction() { return search_up_; }
    QAction* getSearchDownAction() { return search_down_; }

signals:

public slots:
    void goUp();
    void goDown();
    void openSearch();

private slots:
    void resultsChanged();
    void search();

private:
    bool hidable_ = true;

    QAction* start_search_;
    QLineEdit* search_field_;
    QAction* search_up_;
    QAction* search_down_;
    QLabel* search_result_;
    SearchModel* model_ = nullptr;
    int cur_search_index_;

    QPalette orig_search_field_;
    QPalette failed_search_field_;
    // TODO: switches: case, RegEx

    // state
    bool searching_ = false;

    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;
};


#endif // SEARCHWIDGET_H
