#pragma once

#include <string>

#include <QString>

class H5adLoader
{
public:
    void LoadTaxonomy(QString fileName, std::string& taxonomyStr);
    void LoadFile(QString fileName);
};
