#include "Bookmark.h"
#include "Config.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtCore/QDir>
#include <QtCore/QFile>

double Bookmark::getRelevance(const QString &search) const
{
    double res = getMatchTextRelevance(search);

    if (res == -1)
        return res;
    return res;
}

double Bookmark::getMatchTextRelevance(const QString &search) const
{
    if (this->name.contains(search)) {
        return (double)search.size() / (this->name.length() * 8);
    }
    return -1;
}

Bookmark Bookmark::fromJSON(const QJsonObject &obj)
{
    Bookmark bookmark;
    bookmark.name = obj.value(JSONBookmark::Name).toString().toLower();
    bookmark.title = obj.value(JSONBookmark::Title).toString();
    bookmark.description = obj.value(JSONBookmark::Description).toString().toLower();
    return bookmark;
}
