#include "markdownbookmarks.h"
#include "core/Config.h"
#include "core/FileReader.h"

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
{
    Q_UNUSED(args)
    // Add file watcher for config (using default path initially)
    watcher.addPath(Config::bookmarkFilePath());
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
        watcher.addPath(path);
        reloadConfiguration();
    });
}

MarkdownBookmarks::~MarkdownBookmarks()
{
}

void MarkdownBookmarks::reloadConfiguration()
{
    KConfigGroup grp = config();
    FileReader reader(grp);
    bookmarks = reader.getBookmarks();

    // Update file watcher to use the configured path
    const QString newPath = Config::bookmarkFilePath(grp);
    if (!watcher.files().contains(newPath)) {
        // Remove old paths and add new one
        if (!watcher.files().isEmpty()) {
            watcher.removePaths(watcher.files());
        }
        watcher.addPath(newPath);
    }



    QList<KRunner::RunnerSyntax> syntaxes;
    syntaxes.append(KRunner::RunnerSyntax(QStringLiteral("bookmark :q:"), QStringLiteral("Searches for Markdown bookmarks matching the query.")));
    setSyntaxes(syntaxes);
}

void MarkdownBookmarks::match(KRunner::RunnerContext &context)
{
    QString search = context.query();

    KConfigGroup grp = config();
    FileReader reader(grp);
    QList<Bookmark> currentBookmarks = reader.getBookmarks();

    QList<KRunner::QueryMatch> matches;
    if (search.size() > 2 || context.singleRunnerQueryMode()) {
        // Convert search to lowercase for case-insensitive matching
        const QString searchLower = search.toLower();
        for (const auto &bookmark : std::as_const(currentBookmarks)) {
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
    return match;
}


K_PLUGIN_CLASS_WITH_JSON(MarkdownBookmarks, "markdownbookmarks.json")

#include "markdownbookmarks.moc"
#include "moc_markdownbookmarks.cpp"
