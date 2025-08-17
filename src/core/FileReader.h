#ifndef MARKDOWNBOOKMARKS_FILEREADER_H
#define MARKDOWNBOOKMARKS_FILEREADER_H

#include "Bookmark.h"

class QFile;
class KConfigGroup;

class FileReader
{
public:
    /**
     * Initialize reusable variables
     */
    explicit FileReader(const KConfigGroup &config);

    /**
     * Reads the bookmarks from the different files
     * Rewrite of readJSONFile function but more maintainable and ~20% faster
     *
     * @return QList<Bookmark>
     */
    QList<Bookmark> getBookmarks() const;

private:
    const KConfigGroup &configGroup;

    /**
     * Returns list of bookmarks from the given markdown file
     * @param getAllBookmarks
     * @param bookmarkMarkdownFile
     * @return QList<Bookmark>
     */
    QList<Bookmark> parseMarkdownFile(QFile &bookmarkMarkdownFile) const;
};

#endif // MARKDOWNBOOKMARKS_FILEREADER_H
