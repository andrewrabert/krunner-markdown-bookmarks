#include "markdownbookmarks_config.h"
#include "core/macros.h"
#include "utilities.h"
#include <KPluginFactory>
#include <KSharedConfig>
#include <QDir>
#include <QFileDialog>
#include <core/Config.h>
#include <core/FileReader.h>

K_PLUGIN_CLASS(MarkdownBookmarksConfig)

#define itemBookmark(item) item->data(Qt::UserRole).value<Bookmark>()

MarkdownBookmarksConfigForm::MarkdownBookmarksConfigForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

MarkdownBookmarksConfig::MarkdownBookmarksConfig(QObject *parent, const QVariantList &)
    : KCModule(qobject_cast<QWidget *>(parent))
{
    m_ui = new MarkdownBookmarksConfigForm(widget());
    auto *layout = new QGridLayout(widget());
    layout->addWidget(m_ui, 0, 0);

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("markdownbookmarks");
    config.config()->reparseConfiguration();

    FileReader reader(config);
    bookmarks = reader.getBookmarks();
}

MarkdownBookmarksConfig::~MarkdownBookmarksConfig()
{
}

void MarkdownBookmarksConfig::load()
{
    m_ui->markdownFilePathEdit->setText(config.readEntry(Config::MarkdownFilePath, QString()));
    m_ui->fetchFaviconsCheckBox->setChecked(config.readEntry(Config::FetchFavicons, false));
    m_ui->faviconCachePathEdit->setText(config.readEntry(Config::FaviconCachePath, QString()));
    connectSignals();
    markAsChanged();
}

void MarkdownBookmarksConfig::save()
{
    config.writeEntry(Config::MarkdownFilePath, m_ui->markdownFilePathEdit->text());
    config.writeEntry(Config::FetchFavicons, m_ui->fetchFaviconsCheckBox->isChecked());
    config.writeEntry(Config::FaviconCachePath, m_ui->faviconCachePathEdit->text());

    config.sync();
    config.config()->sync();
}

void MarkdownBookmarksConfig::defaults()
{
    m_ui->markdownFilePathEdit->clear();
    m_ui->fetchFaviconsCheckBox->setChecked(false);
    m_ui->faviconCachePathEdit->clear();

    markAsChanged();
}

void MarkdownBookmarksConfig::connectSignals()
{
    connect(m_ui->browseFileButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseMarkdownFile);
    connect(m_ui->browseFaviconCacheButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseFaviconCacheDir);
}

void MarkdownBookmarksConfig::browseMarkdownFile()
{
    const QString currentPath = m_ui->markdownFilePathEdit->text();
    const QString startPath = currentPath.isEmpty() ? QDir::homePath() : currentPath;

    const QString fileName =
        QFileDialog::getOpenFileName(widget(), QStringLiteral("Select Markdown File"), startPath, QStringLiteral("Markdown files (*.md);;All files (*)"));

    if (!fileName.isEmpty()) {
        m_ui->markdownFilePathEdit->setText(fileName);
        markAsChanged();
    }
}

void MarkdownBookmarksConfig::browseFaviconCacheDir()
{
    const QString currentPath = m_ui->faviconCachePathEdit->text();
    const QString startPath = currentPath.isEmpty() ? QDir::homePath() : currentPath;

    const QString dirName = QFileDialog::getExistingDirectory(widget(), QStringLiteral("Select Icon Directory"), startPath);

    if (!dirName.isEmpty()) {
        m_ui->faviconCachePathEdit->setText(dirName);
        markAsChanged();
    }
}

#include "markdownbookmarks_config.moc"
