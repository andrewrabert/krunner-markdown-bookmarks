#include "markdownbookmarks.h"
#include "core/Config.h"
#include "core/FileReader.h"
#include "core/SearchEngineReader.h"

#include <KConfigGroup>
#include <KIO/OpenUrlJob>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QTimer>
#include <QUrl>

MarkdownBookmarks::MarkdownBookmarks(QObject *parent, const KPluginMetaData &pluginMetaData, const QVariantList &args)
    : KRunner::AbstractRunner(parent, pluginMetaData)
    , watcher(this)
    , faviconCache(this)
    , searchEngineFaviconCache(this)
{
    Q_UNUSED(args)
    // Add file watcher for config (using default path initially)
    watcher.addPath(Config::bookmarkFilePath());
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
        watcher.addPath(path);
        reloadConfiguration();
    });
    connect(&faviconCache, &FaviconCache::iconsReady, this, [this](const QHash<QString, QIcon> &icons) {
        faviconsByUrl = icons;
    });
    connect(&searchEngineFaviconCache, &FaviconCache::iconsReady, this, [this](const QHash<QString, QIcon> &icons) {
        searchEngineFaviconsByUrl = icons;
    });

    QList<KRunner::RunnerSyntax> syntaxes;
    syntaxes.append(KRunner::RunnerSyntax(QStringLiteral("bookmark :q:"), QStringLiteral("Searches for Markdown bookmarks matching the query.")));
    syntaxes.append(KRunner::RunnerSyntax(QStringLiteral("!keyword :q:"), QStringLiteral("Searches using a configured search engine.")));
    setSyntaxes(syntaxes);
}

MarkdownBookmarks::~MarkdownBookmarks()
{
}

void MarkdownBookmarks::reloadConfiguration()
{
    KConfigGroup grp = config();
    FileReader reader(grp);
    bookmarks = reader.getBookmarks();

    SearchEngineReader seReader(grp);
    searchEngines = seReader.getSearchEngines();

    searchEngineKeywordIndex.clear();
    for (int i = 0; i < searchEngines.size(); ++i) {
        for (const QString &kw : searchEngines[i].keywords) {
            searchEngineKeywordIndex.insert(kw, i);
        }
    }

    // Update file watcher to use configured paths
    const QStringList watchedFiles = watcher.files();
    if (!watchedFiles.isEmpty()) {
        watcher.removePaths(watchedFiles);
    }
    const QString bookmarkPath = Config::bookmarkFilePath(grp);
    if (!bookmarkPath.isEmpty()) {
        watcher.addPath(bookmarkPath);
    }
    const QString searchEnginesPath = Config::searchEnginesFilePath(grp);
    if (!searchEnginesPath.isEmpty()) {
        watcher.addPath(searchEnginesPath);
    }

    // Bookmark favicons
    const QString cachePath = Config::faviconCachePath(grp);
    const bool fetchEnabled = grp.readEntry(Config::FetchFavicons, false);

    QStringList bookmarkUrls;
    if (!cachePath.isEmpty()) {
        bookmarkUrls.reserve(bookmarks.size());
        for (const auto &b : std::as_const(bookmarks)) {
            bookmarkUrls.append(b.url);
        }
    }

    QMetaObject::invokeMethod(&faviconCache,
                              "loadAndFetch",
                              Qt::QueuedConnection,
                              Q_ARG(QString, cachePath),
                              Q_ARG(QStringList, bookmarkUrls),
                              Q_ARG(bool, fetchEnabled));

    // Search engine favicons
    const QString seCachePath = Config::searchEngineFaviconCachePath(grp);
    const bool seFetchEnabled = grp.readEntry(Config::FetchSearchEngineFavicons, false);

    QStringList seUrls;
    if (!seCachePath.isEmpty()) {
        seUrls.reserve(searchEngines.size());
        for (const auto &e : std::as_const(searchEngines)) {
            seUrls.append(e.urlTemplate);
        }
    }

    QMetaObject::invokeMethod(&searchEngineFaviconCache,
                              "loadAndFetch",
                              Qt::QueuedConnection,
                              Q_ARG(QString, seCachePath),
                              Q_ARG(QStringList, seUrls),
                              Q_ARG(bool, seFetchEnabled));
}

void MarkdownBookmarks::match(KRunner::RunnerContext &context)
{
    QString search = context.query();
    QList<KRunner::QueryMatch> matches;

    // Bang detection for search engines
    {
        const QStringList tokens = search.split(' ', Qt::SkipEmptyParts);
        for (int i = 0; i < tokens.size(); ++i) {
            const QString &token = tokens[i];
            if (token.startsWith('!') && token.size() > 1) {
                const QString keyword = token.mid(1).toLower();
                const auto it = searchEngineKeywordIndex.constFind(keyword);
                if (it != searchEngineKeywordIndex.constEnd()) {
                    QStringList queryParts;
                    for (int j = 0; j < tokens.size(); ++j) {
                        if (j != i)
                            queryParts.append(tokens[j]);
                    }
                    const QString searchQuery = queryParts.join(' ');
                    matches.append(createSearchEngineMatch(searchEngines[it.value()], searchQuery));
                }
                break;
            }
        }
    }

    // Bookmark matching
    if (search.size() > 2 || context.singleRunnerQueryMode()) {
        const QString searchLower = search.toLower();
        for (const auto &bookmark : std::as_const(bookmarks)) {
            const double relevance = bookmark.getRelevance(searchLower);
            if (relevance == -1)
                continue;
            matches.append(createQueryMatch(bookmark, relevance));
        }
    }
    context.addMatches(matches);
}

void MarkdownBookmarks::run(const KRunner::RunnerContext & /*context*/, const KRunner::QueryMatch &match)
{
    QUrl location = match.data().toUrl();
    if (!location.isEmpty()) {
        auto job = new KIO::OpenUrlJob(location);
        job->start();
    }
}

KRunner::QueryMatch MarkdownBookmarks::createQueryMatch(const Bookmark &bookmark, const qreal relevance)
{
    KRunner::QueryMatch match(this);
    match.setText(bookmark.title);
    match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Moderate);
    // match.setSubtext(bookmark.url);
    match.setData(bookmark.url);
    match.setRelevance(relevance);

    const auto it = faviconsByUrl.constFind(bookmark.url);
    if (it != faviconsByUrl.constEnd()) {
        match.setIcon(it.value());
    }

    return match;
}

KRunner::QueryMatch MarkdownBookmarks::createSearchEngineMatch(const SearchEngine &engine, const QString &searchQuery)
{
    KRunner::QueryMatch match(this);
    match.setText(engine.name);
    const QString url = engine.buildUrl(searchQuery);
    match.setSubtext(searchQuery);
    match.setData(QUrl(url));
    match.setRelevance(0.9);
    match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::High);

    const auto it = searchEngineFaviconsByUrl.constFind(engine.urlTemplate);
    if (it != searchEngineFaviconsByUrl.constEnd()) {
        match.setIcon(it.value());
    }

    return match;
}

K_PLUGIN_CLASS_WITH_JSON(MarkdownBookmarks, "markdownbookmarks.json")

#include "markdownbookmarks.moc"
#include "moc_markdownbookmarks.cpp"
