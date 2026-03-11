#include "SearchEngineReader.h"
#include "Config.h"

#include <KConfigGroup>
#include <QFile>

SearchEngineReader::SearchEngineReader(const KConfigGroup &config)
    : configGroup(config)
{
}

QList<SearchEngine> SearchEngineReader::getSearchEngines() const
{
    const QString filePath = Config::searchEnginesFilePath(configGroup);
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        return {};
    }

    QList<SearchEngine> engines;
    const QString content = QString::fromUtf8(file.readAll());
    const QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();

        // Handle markdown list items (- or * followed by space)
        if (line.startsWith("- ") || line.startsWith("* ")) {
            line = line.mid(2).trimmed();
        }

        // Parse [name](url) `keywords`
        if (!line.startsWith('[') || !line.contains("](")) {
            continue;
        }

        const int titleStart = 1;
        const int titleEnd = line.indexOf("](");
        if (titleEnd <= titleStart)
            continue;

        const int urlStart = titleEnd + 2;
        const int urlEnd = line.indexOf(')', urlStart);
        if (urlEnd < 0)
            continue;

        const QString name = line.mid(titleStart, titleEnd - titleStart);
        const QString url = line.mid(urlStart, urlEnd - urlStart);

        // URL must contain {} placeholder
        if (!url.contains(QStringLiteral("{}")))
            continue;

        // Look for backtick-delimited keywords after the link
        const int backtickStart = line.indexOf('`', urlEnd + 1);
        if (backtickStart < 0)
            continue;
        const int backtickEnd = line.indexOf('`', backtickStart + 1);
        if (backtickEnd < 0)
            continue;

        const QString keywordsStr = line.mid(backtickStart + 1, backtickEnd - backtickStart - 1);
        QStringList keywords;
        const QStringList parts = keywordsStr.split(',');
        for (const QString &part : parts) {
            const QString trimmed = part.trimmed().toLower();
            if (!trimmed.isEmpty()) {
                keywords.append(trimmed);
            }
        }

        if (keywords.isEmpty())
            continue;

        SearchEngine engine;
        engine.name = name;
        engine.urlTemplate = url;
        engine.keywords = keywords;
        engines.append(engine);
    }

    return engines;
}
