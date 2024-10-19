#pragma once

#include "LeadFile.h"

#include <string>

#include <QString>

class H5adLoader
{
public:
    void LoadTaxonomy(QString fileName, std::string& taxonomyStr);
    void LoadX();
    void LoadFile(QString fileName);

private:
    LEAD::File lf;
};
