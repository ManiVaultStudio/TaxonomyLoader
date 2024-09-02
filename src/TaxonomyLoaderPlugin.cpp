#include "TaxonomyLoaderPlugin.h"

#include "Taxonomy.h"

#include <Set.h>

#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <QDir>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <unordered_set>

Q_PLUGIN_METADATA(IID "studio.manivault.TaxonomyLoaderPlugin")

using namespace mv;
using namespace mv::gui;

// =============================================================================
// View
// =============================================================================

TaxonomyLoaderPlugin::~TaxonomyLoaderPlugin(void)
{

}

void TaxonomyLoaderPlugin::init()
{

}

void TaxonomyLoaderPlugin::loadData()
{
    //Q_INIT_RESOURCE(met_loader_resources);

    // Load taxonomy
    qDebug() << "Reading taxonomy annotations from file..";
    Taxonomy taxonomy = Taxonomy::fromJsonFile();
    taxonomy.print();
}

QIcon TaxonomyLoaderPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return Application::getIconFont("FontAwesome").getIcon("database");
}

// =============================================================================
// Factory
// =============================================================================

LoaderPlugin* TaxonomyLoaderPluginFactory::produce()
{
    return new TaxonomyLoaderPlugin(this);
}

DataTypes TaxonomyLoaderPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
