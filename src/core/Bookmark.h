#pragma once

#include <QJsonObject>
#include <QString>

class Bookmark
{
public:
    QString name;
    QString title;
    QString description;
    QString url;

    /**
     * Gets the relevance with all parameters and rules
     * @param search
     * @return double
     */
    double getRelevance(const QString &search) const;

    /**
     * Gets the relevance of the compared texts
     * @param search
     * @return  double
     */
    double getMatchTextRelevance(const QString &search) const;

    /**
     * Returns a Bookmark instance based on the data from the JSON object
     * @param obj
     * @return Bookmark
     */
    static Bookmark fromJSON(const QJsonObject &obj);
};

Q_DECLARE_METATYPE(Bookmark)
