#include "paletteTreeWidget.h"

#include <QtGui/QMouseEvent>
#include <QtWidgets/QMenu>

#include "mainwindow/palette/paletteTree.h"
#include "mainwindow/palette/draggableElement.h"
#include "dialogs/metamodelingOnFly/chooseTypeDialog.h"

using namespace qReal;
using namespace gui;

EditorManagerInterface *PaletteTreeWidget::mEditorManager = NULL;

PaletteTreeWidget::PaletteTreeWidget(PaletteTree &palette, MainWindow &mainWindow
		, EditorManagerInterface &editorManagerProxy, bool editable)
	: mMainWindow(mainWindow)
	, mPaletteTree(palette)
	, mEditable(editable)
{
	mEditorManager = &editorManagerProxy;
	mEventFilter = new EventFilterPaletteDraggableElement();
}

void PaletteTreeWidget::addGroups(QList<QPair<QString, QList<PaletteElement>>> &groups
		, QMap<QString, QString> const &descriptions
		, bool hideIfEmpty
		, QString const &diagramFriendlyName
		, bool sort)
{
	if (groups.isEmpty() && hideIfEmpty) {
		hide();
		return;
	}

	clear();
	show();

	if (sort) {
		qSort(groups.begin(), groups.end()
				, [](QPair<QString, QList<PaletteElement>> const &e1
						, QPair<QString, QList<PaletteElement>> const &e2)
					{
						return e1.first < e2.first;
					}
				);
	}

	int expandedCount = 0;
	for (auto &group : groups) {
		QTreeWidgetItem * const item = new QTreeWidgetItem;
		item->setText(0, group.first);
		item->setToolTip(0, descriptions[group.first]);

		if (sort) {
			sortByFriendlyName(group.second);
		}

		addItemsRow(group.second, item);
		addTopLevelItem(item);

		if (SettingsManager::value(diagramFriendlyName + group.first, 0).toBool()) {
			++expandedCount;
			expandItem(item);
		}
	}

	if (expandedCount == 0) {
		expand();
	}
}

void PaletteTreeWidget::addItemType(PaletteElement const &data, QTreeWidgetItem *parent)
{
	QTreeWidgetItem *leaf = new QTreeWidgetItem;
	DraggableElement *element = new DraggableElement(mMainWindow, data
			, mPaletteTree.iconsView(), *mEditorManager);

	parent->addChild(leaf);
	setItemWidget(leaf, 0, element);
	element->installEventFilter(mEventFilter);
}

void PaletteTreeWidget::addItemsRow(QList<PaletteElement> const &items, QTreeWidgetItem *parent)
{
	if (mPaletteTree.itemsCountInARow() == 1 || !mPaletteTree.iconsView()) {
		foreach (PaletteElement const &element, items) {
			addItemType(element, parent);
		}

		return;
	}

	for (QList<PaletteElement>::ConstIterator it = items.begin(); it != items.end();) {
		QWidget *field = new QWidget;
		QHBoxLayout *layout = new QHBoxLayout;
		int count = mPaletteTree.itemsCountInARow();
		for (; it != items.end() && --count > 0; ++it) {
			DraggableElement *element = new DraggableElement(mMainWindow, *it, true, *mEditorManager);
			element->setToolTip((*it).description());
			layout->addWidget(element, count > 0 ? 50 : 0);
			element->installEventFilter(mEventFilter);
		}

		field->setLayout(layout);
		field->setMinimumHeight(80);
		QTreeWidgetItem *leaf = new QTreeWidgetItem;
		parent->addChild(leaf);
		if (mEditable) {
			leaf->setFlags(leaf->flags() | Qt::ItemIsEditable);
		}

		setItemWidget(leaf, 0, field);
	}
}

void PaletteTreeWidget::addElementPaletteActionTriggered()
{
	ChooseTypeDialog *chooseTypeDialog = new ChooseTypeDialog(mMainWindow
			, mPaletteTree.currentEditor(), *mEditorManager);
	chooseTypeDialog->setModal(true);
	chooseTypeDialog->show();
}

void PaletteTreeWidget::groupExpanded(QTreeWidgetItem *item)
{
	int const topLevelIndex = 0;
	QString const name = item->text(topLevelIndex);
	QString const userAction = QString::fromUtf8("Развернуть группу элементов в палитре — название группы: ") + name + QString::fromUtf8("|");
	mPaletteTree.reportPaletteUserAction(userAction);
}

void PaletteTreeWidget::groupCollapsed(QTreeWidgetItem *item)
{
	int const topLevelIndex = 0;
	QString const name = item->text(topLevelIndex);
	QString const userAction = QString::fromUtf8("Свернуть группу элементов в палитре — название группы: ") + name + QString::fromUtf8("|");
	mPaletteTree.reportPaletteUserAction(userAction);
}

void PaletteTreeWidget::elementClicked(QTreeWidgetItem *item, int column)
{
	QString const name = item->text(column);
	QString const userAction = QString::fromUtf8("Нажать на элемент в палитре — название элемента: ") + name + "|";
	mPaletteTree.reportPaletteUserAction(userAction);
}

void PaletteTreeWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		if (mEditorManager->isInterpretationMode()) {
			QMenu menu;
			QAction * const addElementPaletteAction = menu.addAction(tr("Add Element"));
			connect(addElementPaletteAction, SIGNAL(triggered()), this, SLOT(addElementPaletteActionTriggered()));
			menu.exec(QCursor::pos());
		}
	}

	QTreeWidget::mousePressEvent(event);
}

void PaletteTreeWidget::scrollContentsBy(int dx, int dy)
{
	QString const direction = (dy == 1) ? QString::fromUtf8("вверх") : QString::fromUtf8("вниз");
	QString const userAction = QString::fromUtf8("Прокрутить палитру — направление: ") + direction + QString::fromUtf8("|шагов: 1|");
	mPaletteTree.reportPaletteUserAction(userAction);
	QTreeWidget::scrollContentsBy(dx, dy);
}

void PaletteTreeWidget::expand()
{
	for (int i = 0; i < topLevelItemCount(); i++) {
		if (topLevelItem(i)) {
			expandChildren(topLevelItem(i));
		}
	}
}

void PaletteTreeWidget::sortByFriendlyName(IdList &ids)
{
	qSort(ids.begin(), ids.end(), idLessThan);
}

void PaletteTreeWidget::sortByFriendlyName(QList<PaletteElement> &elements)
{
	qSort(elements.begin(), elements.end(), paletteElementLessThan);
}

void PaletteTreeWidget::editItem(QTreeWidgetItem * const item)
{
	edit(indexFromItem(item));
}

void PaletteTreeWidget::addGroupsTracking()
{
	connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(elementClicked(QTreeWidgetItem*,int)));
	connect(this,SIGNAL(itemExpanded(QTreeWidgetItem *)), this,SLOT(groupExpanded(QTreeWidgetItem *)));
	connect(this,SIGNAL(itemCollapsed(QTreeWidgetItem*)), this,SLOT(groupCollapsed(QTreeWidgetItem *)));
}

void PaletteTreeWidget::expandChildren(QTreeWidgetItem *item)
{
	for (int i = 0; i < item->childCount(); i++) {
		if (item->child(i)) {
			expandChildren(item->child(i));
		}
	}

	item->treeWidget()->expandItem(item);
}

void PaletteTreeWidget::collapse()
{
	for (int i = 0; i < topLevelItemCount(); i++) {
		if (topLevelItem(i)) {
			collapseChildren(topLevelItem(i));
		}
	}
}

void PaletteTreeWidget::collapseChildren(QTreeWidgetItem *item)
{
	for (int i = 0; i < item->childCount(); i++) {
		if (item->child(i)) {
			collapseChildren(item->child(i));
		}
	}

	item->treeWidget()->collapseItem(item);
}

bool PaletteTreeWidget::idLessThan(Id const &s1, Id const &s2)
{
	return mEditorManager->friendlyName(s1).toLower() < mEditorManager->friendlyName(s2).toLower();
}

bool PaletteTreeWidget::paletteElementLessThan(PaletteElement const &s1, PaletteElement const &s2)
{
	return idLessThan(s1.id(), s2.id());
}
