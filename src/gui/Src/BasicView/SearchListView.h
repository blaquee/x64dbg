#ifndef SEARCHLISTVIEW_H
#define SEARCHLISTVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include "MenuBuilder.h"
#include "ActionHelpers.h"
#include "AbstractSearchList.h"

class SearchListView : public QWidget, public ActionHelper<SearchListView>
{
    Q_OBJECT

public:
    explicit SearchListView(QWidget* parent, AbstractSearchList* abstractSearchList, bool enableRegex, bool enableLock);

    AbstractStdTable* mCurList;
    QLineEdit* mSearchBox;
    int mSearchStartCol;

    bool findTextInList(AbstractStdTable* list, QString text, int row, int startcol, bool startswith);
    void refreshSearchList();

    bool isSearchBoxLocked();

private slots:
    void searchTextChanged(const QString & text);
    void listContextMenu(const QPoint & pos);
    void doubleClickedSlot();
    void searchSlot();
    void on_checkBoxRegex_stateChanged(int state);
    void on_checkBoxLock_toggled(bool checked);

signals:
    void enterPressedSignal();
    void listContextMenuSignal(QMenu* wMenu);
    void emptySearchResult();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
    QCheckBox* mRegexCheckbox;
    QCheckBox* mLockCheckbox;
    QAction* mSearchAction;

    AbstractSearchList* mAbstractSearchList;
};

#endif // SEARCHLISTVIEW_H
