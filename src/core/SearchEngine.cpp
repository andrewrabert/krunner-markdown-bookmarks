#include "SearchEngine.h"
#include <QUrl>

QString SearchEngine::buildUrl(const QString &query) const
{
    QString url = urlTemplate;
    url.replace(QStringLiteral("{}"), QString::fromUtf8(QUrl::toPercentEncoding(query)));
    return url;
}
