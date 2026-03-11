#pragma once

#include <QString>
#include <QStringList>

struct SearchEngine {
    QString name;
    QString urlTemplate;
    QStringList keywords;

    QString buildUrl(const QString &query) const;
};
