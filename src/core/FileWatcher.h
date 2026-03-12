#pragma once

#include <KDirWatch>
#include <QObject>
#include <QSet>
#include <QString>

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    FileWatcher();
    ~FileWatcher() override;

    void addFile(const QString &path);
    void addDir(const QString &path);
    void removeFile(const QString &path);
    void removeDir(const QString &path);

Q_SIGNALS:
    void fileChanged(const QString &path);
    void dirChanged(const QString &path);

private:
    void onChanged(const QString &path);

    KDirWatch *m_watcher;
    QSet<QString> m_files;
    QSet<QString> m_dirs;
};
