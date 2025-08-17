#ifndef MARKDOWNBOOKMARKS_H
#define MARKDOWNBOOKMARKS_H

#include "core/Bookmark.h"
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
    QFileSystemWatcher watcher;

    KRunner::QueryMatch createQueryMatch(const Bookmark &bookmark, qreal relevance);

public: // Plasma::AbstractRunner API
    void reloadConfiguration() override;
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;
};

#endif
