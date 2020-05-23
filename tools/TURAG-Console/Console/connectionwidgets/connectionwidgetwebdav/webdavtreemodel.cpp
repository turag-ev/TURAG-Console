/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "webdavtreeitem.h"
#include "webdavtreemodel.h"

#include <QStringList>
#include <libs/iconmanager.h>

WebDAVTreeModel::WebDAVTreeModel(QString rootDir, QObject *parent)
    : QAbstractItemModel(parent)
{
	rootItem = new WebDAVTreeItem("/", "/");

	WebDAVTreeItem * rootDir_ = new WebDAVTreeItem(rootDir, rootDir, rootItem);
	rootItem->appendChild(rootDir_);
}

WebDAVTreeModel::~WebDAVTreeModel()
{
    delete rootItem;
}

int WebDAVTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<WebDAVTreeItem*>(parent.internalPointer())->columnCount();
    else
//		return 2;
		return 1;
}

QVariant WebDAVTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
            WebDAVTreeItem *item = static_cast<WebDAVTreeItem*>(index.internalPointer());
            return item->data(index.column());
        }
    case Qt::DecorationRole:
        return IconManager::get("folder");

    default:
        return QVariant();
    }
}

Qt::ItemFlags WebDAVTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant WebDAVTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return "Name";
//		case 1:
//			return "Files";
		}
	}

    return QVariant();
}

QModelIndex WebDAVTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    WebDAVTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<WebDAVTreeItem*>(parent.internalPointer());

    WebDAVTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex WebDAVTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    WebDAVTreeItem *childItem = static_cast<WebDAVTreeItem*>(index.internalPointer());
    WebDAVTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int WebDAVTreeModel::rowCount(const QModelIndex &parent) const
{
    WebDAVTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<WebDAVTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void WebDAVTreeModel::addRows(const QList<WebDAVTreeItem *> &children, const QModelIndex &parent) {
	WebDAVTreeItem *parentItem;
	if (parent.isValid()) {
		parentItem = static_cast<WebDAVTreeItem*>(parent.internalPointer());
	} else {
		parentItem = rootItem;
	}

	beginInsertRows(parent, parentItem->childCount(), parentItem->childCount() + children.size());
	for (WebDAVTreeItem * child : children) {
		child->setParent(parentItem);
		parentItem->appendChild(child);
	}
	endInsertRows();
}

bool WebDAVTreeModel::hasChildren(const QModelIndex & parent) const {
	if (parent.isValid()) {
		WebDAVTreeItem* item = static_cast<WebDAVTreeItem*>(parent.internalPointer());
		if (item->wasExpanded()) {
			return item->childCount() > 0;
		} else {
			return true;
		}
	}
	return true;
}

void WebDAVTreeModel::clear(const QModelIndex &parent) {
	WebDAVTreeItem *parentItem;
	if (parent.isValid()) {
		parentItem = static_cast<WebDAVTreeItem*>(parent.internalPointer());
	} else {
		parentItem = rootItem;
	}
	if (parentItem->childCount() > 0) {
		beginRemoveRows(parent, 0, parentItem->childCount() - 1);
		parentItem->deleteChildren();
		endRemoveRows();
	}
	parentItem->clearFiles();
}
