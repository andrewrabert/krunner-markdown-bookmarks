#include "markdownbookmarks.h"
#include "core/Config.h"
#include "core/FileReader.h"
#include "core/SearchEngineReader.h"

#include <KApplicationTrader>
#include <KConfigGroup>
#include <KIO/CommandLauncherJob>
#include <KIO/OpenUrlJob>
#include <KSharedConfig>
#include <KShell>
#include <KSycoca>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QIcon>
#include <QLoggingCategory>
#include <QUrl>

Q_LOGGING_CATEGORY(LOG_BOOKMARKS, "krunner.markdownbookmarks")

MarkdownBookmarks::MarkdownBookmarks(QObject *parent, const KPluginMetaData &pluginMetaData, const QVariantList &args)
    : KRunner::AbstractRunner(parent, pluginMetaData)
    , watcher(new FileWatcher())
    , faviconCache(watcher, this)
    , searchEngineFaviconCache(watcher, this)
{
    Q_UNUSED(args)

    connect(watcher, &FileWatcher::fileChanged, this, [this](const QString &path) {
        if (!watchedConfigFiles.contains(path))
            return;
        qCInfo(LOG_BOOKMARKS) << "File changed, reloading:" << path;
        reloadConfiguration();
    });
    connect(&faviconCache, &FaviconCache::iconsReady, this, [this](const QHash<QString, QIcon> &icons) {
        faviconsByUrl = icons;
    });
    connect(&searchEngineFaviconCache, &FaviconCache::iconsReady, this, [this](const QHash<QString, QIcon> &icons) {
        searchEngineFaviconsByUrl = icons;
    });
    connect(KSycoca::self(), &KSycoca::databaseChanged, this, &MarkdownBookmarks::configurePrivateBrowsingAction);

    QList<KRunner::RunnerSyntax> syntaxes;
    syntaxes.append(KRunner::RunnerSyntax(QStringLiteral("bookmark :q:"), QStringLiteral("Searches for Markdown bookmarks matching the query.")));
    syntaxes.append(KRunner::RunnerSyntax(QStringLiteral("!keyword :q:"), QStringLiteral("Searches using a configured search engine.")));
    setSyntaxes(syntaxes);
}

MarkdownBookmarks::~MarkdownBookmarks()
{
    delete watcher;
}

void MarkdownBookmarks::init()
{
    configurePrivateBrowsingAction();
    reloadConfiguration();
}

void MarkdownBookmarks::configurePrivateBrowsingAction()
{
    m_searchEngineActions.clear();
    m_privateAction = KServiceAction();

    KService::Ptr service = KApplicationTrader::preferredService(QStringLiteral("x-scheme-handler/http"));
    if (!service) {
        return;
    }
    const auto actions = service->actions();
    for (const auto &action : actions) {
        const bool containsPrivate = action.text().contains(QLatin1String("private"), Qt::CaseInsensitive);
        const bool containsIncognito = action.text().contains(QLatin1String("incognito"), Qt::CaseInsensitive);
        if (containsPrivate || containsIncognito) {
            m_privateAction = action;
            const QString iconName = QIcon::fromTheme(QStringLiteral("view-private"), QIcon::fromTheme(QStringLiteral("view-hidden"))).name();
            const QString text = containsPrivate ? QStringLiteral("Search in private window") : QStringLiteral("Search in incognito window");
            m_searchEngineActions = {KRunner::Action(action.exec(), iconName, text)};
            return;
        }
    }
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

    // Resolve global search engine keywords to indices
    globalSearchEngineIndices.clear();
    const QString globalStr = grp.readEntry(Config::GlobalSearchEngines, QString());
    if (!globalStr.isEmpty()) {
        const QStringList parts = globalStr.split(',');
        QSet<int> seen;
        for (const QString &part : parts) {
            const QString kw = part.trimmed().toLower();
            const auto it = searchEngineKeywordIndex.constFind(kw);
            if (it != searchEngineKeywordIndex.constEnd() && !seen.contains(it.value())) {
                seen.insert(it.value());
                globalSearchEngineIndices.append(it.value());
            }
        }
    }

    // Update file watches for bookmark and search engine files
    const QString bookmarkPath = Config::bookmarkFilePath(grp);
    const QString searchEnginePath = Config::searchEnginesFilePath(grp);

    watchedConfigFiles.clear();
    if (!bookmarkPath.isEmpty())
        watchedConfigFiles.append(bookmarkPath);
    if (!searchEnginePath.isEmpty())
        watchedConfigFiles.append(searchEnginePath);

    watcher->addFile(bookmarkPath);
    watcher->addFile(searchEnginePath);

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
    bool bangDetected = false;
    {
        const QStringList tokens = search.split(' ', Qt::SkipEmptyParts);
        for (int i = 0; i < tokens.size(); ++i) {
            const QString &token = tokens[i];
            if (token.startsWith('!') && token.size() > 1) {
                const QString keyword = token.mid(1).toLower();
                const auto it = searchEngineKeywordIndex.constFind(keyword);
                if (it != searchEngineKeywordIndex.constEnd()) {
                    bangDetected = true;
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

        // Global search engines (skipped when bang detected)
        if (!bangDetected) {
            for (int i = 0; i < globalSearchEngineIndices.size(); ++i) {
                const qreal relevance = 0.1 / (i + 1);
                matches.append(
                    createSearchEngineMatch(searchEngines[globalSearchEngineIndices[i]], search, relevance, KRunner::QueryMatch::CategoryRelevance::Low));
            }
        }
    }
    context.addMatches(matches);
}

void MarkdownBookmarks::run(const KRunner::RunnerContext & /*context*/, const KRunner::QueryMatch &match)
{
    QUrl location = match.data().toUrl();
    if (location.isEmpty()) {
        return;
    }

    if (match.selectedAction()) {
        const QString privateExec = m_privateAction.exec();
        QString command;
        if (privateExec.contains(QLatin1String("%u")) || privateExec.contains(QLatin1String("%U"))) {
            command = privateExec;
            command.replace(QLatin1String("%u"), KShell::quoteArg(location.toString()));
            command.replace(QLatin1String("%U"), KShell::quoteArg(location.toString()));
        } else {
            command = privateExec + QLatin1Char(' ') + KShell::quoteArg(location.toString());
        }
        auto *job = new KIO::CommandLauncherJob(command);
        job->start();
    } else {
        auto *job = new KIO::OpenUrlJob(location);
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

KRunner::QueryMatch MarkdownBookmarks::createSearchEngineMatch(const SearchEngine &engine,
                                                               const QString &searchQuery,
                                                               const qreal relevance,
                                                               const KRunner::QueryMatch::CategoryRelevance categoryRelevance)
{
    KRunner::QueryMatch match(this);
    match.setText(engine.name);
    const QString url = engine.buildUrl(searchQuery);
    match.setSubtext(searchQuery);
    match.setData(QUrl(url));
    match.setRelevance(relevance);
    match.setCategoryRelevance(categoryRelevance);

    if (!m_searchEngineActions.isEmpty()) {
        match.setActions(m_searchEngineActions);
    }

    const auto it = searchEngineFaviconsByUrl.constFind(engine.urlTemplate);
    if (it != searchEngineFaviconsByUrl.constEnd()) {
        match.setIcon(it.value());
    }

    return match;
}

K_PLUGIN_CLASS_WITH_JSON(MarkdownBookmarks, "markdownbookmarks.json")

#include "markdownbookmarks.moc"
#include "moc_markdownbookmarks.cpp"
