#pragma once

#include "SearchEngine.h"
#include <QList>

class KConfigGroup;

class SearchEngineReader
{
public:
    explicit SearchEngineReader(const KConfigGroup &config);

    QList<SearchEngine> getSearchEngines() const;

private:
    const KConfigGroup &configGroup;
};
