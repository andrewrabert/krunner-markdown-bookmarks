#include "FaviconCache.h"
#include "FileWatcher.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QNetworkReply>
#include <QUrl>

FaviconCache::FaviconCache(FileWatcher *watcher, QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_watcher(watcher)
{
    connect(m_watcher, &FileWatcher::fileChanged, this, [this](const QString &path) {
        const QString filename = QFileInfo(path).completeBaseName();
        if (m_iconCache.remove(filename)) {
            resolveAndEmit();
        }
    });
    connect(m_watcher, &FileWatcher::dirChanged, this, [this](const QString &path) {
        if (path == m_cacheDir) {
            clearFileWatches();
            m_iconCache.clear();
            resolveAndEmit();
        }
    });
}

void FaviconCache::setCacheDir(const QString &dir)
{
    if (dir == m_cacheDir)
        return;
    clearFileWatches();
    m_cacheDir = dir;
    m_iconCache.clear();
}

void FaviconCache::clearFileWatches()
{
    for (const QString &f : std::as_const(m_watchedFiles)) {
        m_watcher->removeFile(f);
    }
    m_watchedFiles.clear();
}

QString FaviconCache::cacheFilePath(const QString &domain) const
{
    return m_cacheDir + QLatin1Char('/') + domain + QStringLiteral(".png");
}

QString FaviconCache::domainFromUrl(const QString &url)
{
    QUrl parsed(url);
    if (parsed.host().isEmpty()) {
        parsed = QUrl(QStringLiteral("https://") + url);
    }
    return parsed.host().toLower();
}

QString FaviconCache::providerUrl(const QString &domain, Provider provider)
{
    switch (provider) {
    case Provider::DuckDuckGo:
        return QStringLiteral("https://icons.duckduckgo.com/ip3/%1.ico").arg(domain);
    case Provider::Google:
        return QStringLiteral("https://www.google.com/s2/favicons?domain=%1&sz=%2").arg(domain, QString::number(ICON_SIZE));
    }
    return {};
}

QIcon FaviconCache::iconForUrl(const QString &url) const
{
    if (m_cacheDir.isEmpty())
        return {};

    const QString domain = domainFromUrl(url);
    if (domain.isEmpty())
        return {};

    const auto it = m_iconCache.constFind(domain);
    if (it != m_iconCache.constEnd())
        return it.value();

    const QString path = cacheFilePath(domain);
    const QIcon icon(path);
    if (!icon.isNull() && !icon.availableSizes().isEmpty()) {
        m_iconCache.insert(domain, icon);
        m_watcher->addFile(path);
        m_watchedFiles.insert(path);
        return icon;
    }

    return {};
}

void FaviconCache::loadAndFetch(const QString &cachePath, const QStringList &urls, bool fetchEnabled)
{
    const QString oldDir = m_cacheDir;
    setCacheDir(cachePath);
    m_urls = urls;

    if (m_cacheDir != oldDir) {
        if (!oldDir.isEmpty()) {
            m_watcher->removeDir(oldDir);
        }
        if (!m_cacheDir.isEmpty()) {
            QDir().mkpath(m_cacheDir);
            m_watcher->addDir(m_cacheDir);
        }
    }

    if (fetchEnabled && !m_urls.isEmpty()) {
        fetchMissing();
    }

    resolveAndEmit();
}

void FaviconCache::resolveAndEmit()
{
    QHash<QString, QIcon> icons;
    for (const auto &url : std::as_const(m_urls)) {
        const QIcon icon = iconForUrl(url);
        if (!icon.isNull()) {
            icons.insert(url, icon);
        }
    }
    Q_EMIT iconsReady(icons);
}

void FaviconCache::fetchMissing()
{
    if (m_cacheDir.isEmpty())
        return;

    QSet<QString> seen;
    for (const auto &url : std::as_const(m_urls)) {
        const QString domain = domainFromUrl(url);
        if (domain.isEmpty() || seen.contains(domain))
            continue;
        seen.insert(domain);

        if (m_iconCache.contains(domain) || QFile::exists(cacheFilePath(domain)))
            continue;

        fetchFavicon(domain, Provider::DuckDuckGo);
    }
}

void FaviconCache::fetchFavicon(const QString &domain, Provider provider)
{
    QNetworkRequest request(QUrl(providerUrl(domain, provider)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(10000);

    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, domain, provider]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            tryNextProvider(domain, provider);
            return;
        }

        const QByteArray data = reply->readAll();
        if (data.isEmpty()) {
            tryNextProvider(domain, provider);
            return;
        }

        QImage image;
        if (!image.loadFromData(data)) {
            tryNextProvider(domain, provider);
            return;
        }

        if (image.width() != ICON_SIZE || image.height() != ICON_SIZE) {
            image = image.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        const QString path = cacheFilePath(domain);
        if (image.save(path, "PNG")) {
            m_iconCache.insert(domain, QIcon(path));
        }
    });
}

void FaviconCache::tryNextProvider(const QString &domain, Provider failedProvider)
{
    switch (failedProvider) {
    case Provider::DuckDuckGo:
        fetchFavicon(domain, Provider::Google);
        break;
    case Provider::Google:
        break;
    }
}
