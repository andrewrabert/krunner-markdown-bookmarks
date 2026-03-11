#ifndef MARKDOWNBOOKMARKS_H
#define MARKDOWNBOOKMARKS_H

#include "core/Bookmark.h"
#include "core/FaviconCache.h"
#include "core/SearchEngine.h"
#include <KRunner/AbstractRunner>
#include <KRunner/Action>
#include <KServiceAction>
#include <QFileSystemWatcher>

class MarkdownBookmarks : public KRunner::AbstractRunner
{
    Q_OBJECT
public:
    MarkdownBookmarks(QObject *parent, const KPluginMetaData &pluginMetaData, const QVariantList &args);
    ~MarkdownBookmarks() override;

    const QRegularExpression prefixRegex{QStringLiteral(R"(^bookmark(?: +(.*))?$)")};
    QList<Bookmark> bookmarks;
    QList<SearchEngine> searchEngines;
    QHash<QString, int> searchEngineKeywordIndex;
    QFileSystemWatcher watcher;
    FaviconCache faviconCache;
    QHash<QString, QIcon> faviconsByUrl;
    FaviconCache searchEngineFaviconCache;
    QHash<QString, QIcon> searchEngineFaviconsByUrl;

    KRunner::QueryMatch createQueryMatch(const Bookmark &bookmark, qreal relevance);
    KRunner::QueryMatch createSearchEngineMatch(const SearchEngine &engine, const QString &searchQuery);

public: // Plasma::AbstractRunner API
    void reloadConfiguration() override;
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
};

#endif
