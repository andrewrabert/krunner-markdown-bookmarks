#include "markdownbookmarks_config.h"
#include "core/macros.h"
#include "utilities.h"
#include <KPluginFactory>
#include <KSharedConfig>
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
    connectSignals();
    markAsChanged();
}

void MarkdownBookmarksConfig::save()
{
    // Save general settings
    config.writeEntry(Config::MarkdownFilePath, m_ui->markdownFilePathEdit->text());

    config.sync();
    config.config()->sync();
}

void MarkdownBookmarksConfig::defaults()
{
    m_ui->markdownFilePathEdit->clear();

    markAsChanged();
}

void MarkdownBookmarksConfig::connectSignals()
{
    // Connect slots for filters
    // Browse button for custom file path
    connect(m_ui->browseFileButton, &QPushButton::clicked, this, &MarkdownBookmarksConfig::browseMarkdownFile);
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


#include "markdownbookmarks_config.moc"
