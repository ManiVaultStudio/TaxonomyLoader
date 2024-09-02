#pragma once

#include <LoaderPlugin.h>

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <TextData/TextData.h>

#include <Task.h>

using namespace mv::plugin;

// =============================================================================
// Loading input box
// =============================================================================

class TaxonomyLoaderPlugin;

// =============================================================================
// View
// =============================================================================

class TaxonomyLoaderPlugin : public LoaderPlugin
{
    Q_OBJECT
public:
    TaxonomyLoaderPlugin(const PluginFactory* factory) :
        LoaderPlugin(factory)
    { }

    ~TaxonomyLoaderPlugin(void) override;

    void init() override;

    void loadData() Q_DECL_OVERRIDE;

private:

};


// =============================================================================
// Factory
// =============================================================================

class TaxonomyLoaderPluginFactory : public LoaderPluginFactory
{
    Q_INTERFACES(mv::plugin::LoaderPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.TaxonomyLoaderPlugin"
                      FILE  "TaxonomyLoaderPlugin.json")

public:
    TaxonomyLoaderPluginFactory(void) {}
    ~TaxonomyLoaderPluginFactory(void) override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    LoaderPlugin* produce() override;

    mv::DataTypes supportedDataTypes() const override;
};
