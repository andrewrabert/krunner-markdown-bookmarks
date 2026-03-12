#include "FileWatcher.h"

FileWatcher::FileWatcher()
    : QObject(nullptr)
    , m_watcher(new KDirWatch())
{
    connect(m_watcher, &KDirWatch::dirty, this, &FileWatcher::onChanged);
    connect(m_watcher, &KDirWatch::created, this, &FileWatcher::onChanged);
}

FileWatcher::~FileWatcher()
{
    delete m_watcher;
}

void FileWatcher::addFile(const QString &path)
{
    m_files.insert(path);
    m_watcher->addFile(path);
}

void FileWatcher::addDir(const QString &path)
{
    m_dirs.insert(path);
    m_watcher->addDir(path);
}

void FileWatcher::removeFile(const QString &path)
{
    m_files.remove(path);
    m_watcher->removeFile(path);
}

void FileWatcher::removeDir(const QString &path)
{
    m_dirs.remove(path);
    m_watcher->removeDir(path);
}

void FileWatcher::onChanged(const QString &path)
{
    if (m_dirs.contains(path)) {
        Q_EMIT dirChanged(path);
    }
    if (m_files.contains(path)) {
        Q_EMIT fileChanged(path);
    }
}
