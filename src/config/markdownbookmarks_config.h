#ifndef MarkdownBookmarksCONFIG_H
#define MarkdownBookmarksCONFIG_H

#include "ui_markdownbookmarks_config.h"
#include <KCModule>
#include <KConfigGroup>

class MarkdownBookmarksConfigForm : public QWidget, public Ui::MarkdownBookmarksConfigUi
{
    Q_OBJECT

public:
    explicit MarkdownBookmarksConfigForm(QWidget *parent);
};

class MarkdownBookmarksConfig : public KCModule
{
    Q_OBJECT

private:
    MarkdownBookmarksConfigForm *m_ui;
    KConfigGroup config;

public:
    explicit MarkdownBookmarksConfig(QObject *parent, const QVariantList &args);

    ~MarkdownBookmarksConfig() override;

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;

    void connectSignals();

    void browseMarkdownFile();

    void browseSearchEnginesFile();

    void browseFaviconCacheDir();

    void browseSearchEngineFaviconCacheDir();

private:
    void browseForFile(QLineEdit *lineEdit, const QString &dialogTitle);
    void browseForDir(QLineEdit *lineEdit, const QString &dialogTitle);
};

#endif
