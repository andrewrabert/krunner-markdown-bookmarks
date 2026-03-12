#pragma once

#include <QHash>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSet>
#include <QString>

class FileWatcher;

class FaviconCache : public QObject
{
    Q_OBJECT
public:
    explicit FaviconCache(FileWatcher *watcher, QObject *parent = nullptr);

Q_SIGNALS:
    void iconsReady(const QHash<QString, QIcon> &iconsByUrl);

public Q_SLOTS:
    void loadAndFetch(const QString &cachePath, const QStringList &urls, bool fetchEnabled);

private:
    enum class Provider {
        DuckDuckGo, // https://icons.duckduckgo.com/ip3/{domain}.ico
        Google, // https://www.google.com/s2/favicons?domain={domain}&sz=32
    };

    void setCacheDir(const QString &dir);
    void clearFileWatches();
    void resolveAndEmit();
    QIcon iconForUrl(const QString &url) const;
    void fetchMissing();
    void fetchFavicon(const QString &domain, Provider provider);
    void tryNextProvider(const QString &domain, Provider failedProvider);
    static QString providerUrl(const QString &domain, Provider provider);
    QString cacheFilePath(const QString &domain) const;
    static QString domainFromUrl(const QString &url);

    QNetworkAccessManager *m_nam;
    FileWatcher *m_watcher;
    QStringList m_urls;
    mutable QHash<QString, QIcon> m_iconCache;
    mutable QSet<QString> m_watchedFiles;
    QString m_cacheDir;
    static constexpr int ICON_SIZE = 32;
};
