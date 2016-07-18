#include "deditorwidgetmenu.h"

#include <DAction>

#include <QGuiApplication>
#include <QClipboard>

DEditorWidgetMenu::DEditorWidgetMenu(QLineEdit *parent)
    : DMenu(parent)
    , lineEdit(parent)
{
    init(parent);
}

DEditorWidgetMenu::DEditorWidgetMenu(QTextEdit *parent)
    : DMenu(parent)
    , textEdit(parent)
{
    init(parent);
}

void DEditorWidgetMenu::init(QWidget *obj)
{
    if (!obj)
        return;

    DAction *action_undo = addAction(tr("Undo"));
    DAction *action_redo = addAction(tr("Redo"));
    addAction(QString());
    DAction *action_cut = addAction(tr("Cut"));
    DAction *action_copy = addAction(tr("Copy"));
    DAction *action_paste = addAction(tr("Paste"));
    addAction((QString()));
    DAction *action_selectAll = addAction(tr("Select All"));

    connect(action_undo, SIGNAL(triggered(bool)), obj, SLOT(undo()));
    connect(action_redo, SIGNAL(triggered(bool)), obj, SLOT(redo()));
    connect(action_cut, SIGNAL(triggered(bool)), obj, SLOT(cut()));
    connect(action_copy, SIGNAL(triggered(bool)), obj, SLOT(copy()));
    connect(action_paste, SIGNAL(triggered(bool)), obj, SLOT(paste()));
    connect(action_selectAll, SIGNAL(triggered(bool)), obj, SLOT(selectAll()));

    obj->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(obj, &QWidget::customContextMenuRequested,
            this, [this, action_undo, action_redo, action_cut,
            action_copy, action_paste, action_selectAll] {

        if (lineEdit) {
            action_undo->setEnabled(lineEdit->isUndoAvailable());
            action_redo->setEnabled(lineEdit->isRedoAvailable());
            action_cut->setEnabled(lineEdit->hasSelectedText());
            action_paste->setDisabled(qApp->clipboard()->text().isEmpty());
            action_selectAll->setDisabled(lineEdit->text().isEmpty());
        } else if (textEdit) {
            action_undo->setEnabled(textEdit->document()->availableUndoSteps() > 0);
            action_redo->setEnabled(textEdit->document()->availableRedoSteps() > 0);
            action_cut->setDisabled(textEdit->textCursor().selectedText().isEmpty());
            action_paste->setEnabled(textEdit->canPaste());
            action_selectAll->setDisabled(textEdit->toPlainText().isEmpty());
        }

        action_copy->setEnabled(action_cut->isEnabled());

        exec();
    });
}
