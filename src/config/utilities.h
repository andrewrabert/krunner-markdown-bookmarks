#ifndef MARKDOWNBOOKMARKS_UTILITIES_H
#define MARKDOWNBOOKMARKS_UTILITIES_H

#include "core/Bookmark.h"
#include <QListWidgetItem>

namespace Utilities
{
/**
 * Returns a QListWidgetItem with the data of the bookmark
 * @return QListWidgetItem
 */
inline QListWidgetItem *toListWidgetItem(const Bookmark &bookmark)
{
    auto *item = new QListWidgetItem(bookmark.title + " " + bookmark.name);
    item->setData(Qt::UserRole, QVariant::fromValue(bookmark));
    return item;
}
}

#endif // MARKDOWNBOOKMARKS_UTILITIES_H
