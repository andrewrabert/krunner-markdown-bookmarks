#ifndef MARKDOWNBOOKMARKS_CONFIG_H
#define MARKDOWNBOOKMARKS_CONFIG_H

#include <QDir>
#include <QString>
#include <KConfigGroup>

namespace
{
struct Config {
    // General config keys
    constexpr static const auto ConfigFileName = "markdownbookmarksrc";
    constexpr static const auto RootGroup = "Config";
    constexpr static const auto SearchByDescription = "searchByDescription";
    constexpr static const auto MarkdownFilePath = "MarkdownFilePath";

    static inline QString bookmarkFilePath()
    {
        return "";
    }

    static inline QString bookmarkFilePath(const KConfigGroup &config)
    {
        const QString customPath = config.readEntry(MarkdownFilePath, QString());
        if (!customPath.isEmpty()) {
            return customPath;
        }
        return bookmarkFilePath();
    }
};

// Keys of the bookmark json object
struct JSONBookmark {
    constexpr static const auto Id = "id";
    constexpr static const auto Name = "name";
    constexpr static const auto Title = "title";
    constexpr static const auto Description = "description";
    constexpr static const auto UnicodeVersion = "unicode_version";
    constexpr static const auto IosVersion = "ios_version";
};
}
#endif // MARKDOWNBOOKMARKS_CONFIG_H
