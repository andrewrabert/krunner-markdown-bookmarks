#include "FileReader.h"
#include <core/Config.h>

#include <KConfigGroup>
#include <KSharedConfig>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>

FileReader::FileReader(const KConfigGroup &config)
    : configGroup(config)
{
}

QList<Bookmark> FileReader::getBookmarks() const
{
    // Only read custom bookmarks using configurable path
    QFile customBookmarks(Config::bookmarkFilePath(configGroup));
    if (customBookmarks.exists() && customBookmarks.open(QFile::ReadOnly)) {
        return parseMarkdownFile(customBookmarks);
    }

    return QList<Bookmark>();
}

QList<Bookmark> FileReader::parseMarkdownFile(QFile &bookmarkMarkdownFile) const
{
    QList<Bookmark> bookmarks;

    const QString content = QString::fromUtf8(bookmarkMarkdownFile.readAll());
    const QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    for (int lineNum = 0; lineNum < lines.size(); ++lineNum) {
        const QString line = lines[lineNum].trimmed();

        // Parse markdown links [title](target) - both standalone and in lists
        QString linkLine = line;

        // Handle markdown list items (- or * followed by space)
        if (line.startsWith("- ") || line.startsWith("* ")) {
            linkLine = line.mid(2).trimmed();
        }

        if (linkLine.startsWith("[") && linkLine.contains("](") && linkLine.endsWith(")")) {
            const int titleStart = 1;
            const int titleEnd = linkLine.indexOf("](");
            const int targetStart = titleEnd + 2;
            const int targetEnd = linkLine.lastIndexOf(")");

            if (titleEnd > titleStart && targetEnd > targetStart) {
                const QString title = linkLine.mid(titleStart, titleEnd - titleStart);
                const QString target = linkLine.mid(targetStart, targetEnd - targetStart);

                Bookmark newBookmark;
                newBookmark.title = title;
                newBookmark.description = target;
                newBookmark.url = target; // Store target as URL
                newBookmark.name = title.toLower(); // Use title as name for search (lowercase for matching)

                bookmarks.append(newBookmark);
            }
        }
    }

    return bookmarks;
}
