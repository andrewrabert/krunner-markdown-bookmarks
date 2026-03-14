#include "markdownbookmarks_config.h"
#include "utilities.h"
#include <KPluginFactory>
#include <KSharedConfig>
#include <QDir>
#include <QFileDialog>
#include <core/Config.h>

K_PLUGIN_CLASS(MarkdownBookmarksConfig)

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

    connectSignals();
}

MarkdownBookmarksConfig::~MarkdownBookmarksConfig()
{
}

void MarkdownBookmarksConfig::load()
{
    m_ui->markdownFilePathEdit->setText(config.readEntry(Config::MarkdownFilePath, QString()));
    m_ui->searchEnginesFilePathEdit->setText(config.readEntry(Config::SearchEnginesFilePath, QString()));
    m_ui->globalSearchEnginesEdit->setText(config.readEntry(Config::GlobalSearchEngines, QString()));
    m_ui->fetchFaviconsCheckBox->setChecked(config.readEntry(Config::FetchFavicons, false));
    m_ui->faviconCachePathEdit->setText(config.readEntry(Config::FaviconCachePath, QString()));
    m_ui->fetchSearchEngineFaviconsCheckBox->setChecked(config.readEntry(Config::FetchSearchEngineFavicons, false));
    m_ui->searchEngineFaviconCachePathEdit->setText(config.readEntry(Config::SearchEngineFaviconCachePath, QString()));
    markAsChanged();
}

void MarkdownBookmarksConfig::save()
{
    config.writeEntry(Config::MarkdownFilePath, m_ui->markdownFilePathEdit->text());
    config.writeEntry(Config::SearchEnginesFilePath, m_ui->searchEnginesFilePathEdit->text());
    config.writeEntry(Config::GlobalSearchEngines, m_ui->globalSearchEnginesEdit->text());
    config.writeEntry(Config::FetchFavicons, m_ui->fetchFaviconsCheckBox->isChecked());
    config.writeEntry(Config::FaviconCachePath, m_ui->faviconCachePathEdit->text());
    config.writeEntry(Config::FetchSearchEngineFavicons, m_ui->fetchSearchEngineFaviconsCheckBox->isChecked());
    config.writeEntry(Config::SearchEngineFaviconCachePath, m_ui->searchEngineFaviconCachePathEdit->text());

    config.sync();
}

void MarkdownBookmarksConfig::defaults()
{
    m_ui->markdownFilePathEdit->clear();
    m_ui->searchEnginesFilePathEdit->clear();
    m_ui->globalSearchEnginesEdit->clear();
    m_ui->fetchFaviconsCheckBox->setChecked(false);
    m_ui->faviconCachePathEdit->clear();
    m_ui->fetchSearchEngineFaviconsCheckBox->setChecked(false);
    m_ui->searchEngineFaviconCachePathEdit->clear();

    markAsChanged();
}

void MarkdownBookmarksConfig::connectSignals()
{
    connect(m_ui->browseFileButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseMarkdownFile);
    connect(m_ui->browseSearchEnginesButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseSearchEnginesFile);
    connect(m_ui->browseFaviconCacheButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseFaviconCacheDir);
    connect(m_ui->browseSearchEngineFaviconCacheButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseSearchEngineFaviconCacheDir);
}

void MarkdownBookmarksConfig::browseMarkdownFile()
{
    browseForFile(m_ui->markdownFilePathEdit, QStringLiteral("Select Markdown File"));
}

void MarkdownBookmarksConfig::browseSearchEnginesFile()
{
    browseForFile(m_ui->searchEnginesFilePathEdit, QStringLiteral("Select Search Engines File"));
}

void MarkdownBookmarksConfig::browseForFile(QLineEdit *lineEdit, const QString &dialogTitle)
{
    const QString currentPath = lineEdit->text();
    const QString startPath = currentPath.isEmpty() ? QDir::homePath() : currentPath;

    const QString fileName = QFileDialog::getOpenFileName(widget(), dialogTitle, startPath, QStringLiteral("Markdown files (*.md);;All files (*)"));

    if (!fileName.isEmpty()) {
        lineEdit->setText(fileName);
        markAsChanged();
    }
}

void MarkdownBookmarksConfig::browseFaviconCacheDir()
{
    browseForDir(m_ui->faviconCachePathEdit, QStringLiteral("Select Icon Directory"));
}

void MarkdownBookmarksConfig::browseSearchEngineFaviconCacheDir()
{
    browseForDir(m_ui->searchEngineFaviconCachePathEdit, QStringLiteral("Select Icon Directory"));
}

void MarkdownBookmarksConfig::browseForDir(QLineEdit *lineEdit, const QString &dialogTitle)
{
    const QString currentPath = lineEdit->text();
    const QString startPath = currentPath.isEmpty() ? QDir::homePath() : currentPath;

    const QString dirName = QFileDialog::getExistingDirectory(widget(), dialogTitle, startPath);

    if (!dirName.isEmpty()) {
        lineEdit->setText(dirName);
        markAsChanged();
    }
}

#include "markdownbookmarks_config.moc"
