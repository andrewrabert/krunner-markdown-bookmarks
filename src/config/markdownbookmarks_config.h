#ifndef MarkdownBookmarksCONFIG_H
#define MarkdownBookmarksCONFIG_H

#include "ui_markdownbookmarks_config.h"
#include <KCModule>
#include <KConfigGroup>
#include <core/Bookmark.h>

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
    bool customEntriesExist = false;
    bool customFilterWasChecked = false;

    QList<Bookmark> bookmarks;

public:
    explicit MarkdownBookmarksConfig(QObject *parent, const QVariantList &args);

    ~MarkdownBookmarksConfig() override;

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;

    /**
     * Connect all the signals and slots
     */
    void connectSignals();

    /**
     * Open file dialog to browse for Markdown files
     */
    void browseMarkdownFile();
};

#endif
