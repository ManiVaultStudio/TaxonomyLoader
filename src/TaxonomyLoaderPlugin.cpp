#include "TaxonomyLoaderPlugin.h"

#include "H5adLoader.h"

#include "CasTaxonomyData/CasTaxonomyData.h"
#include "CasTaxonomyData/Taxonomy.h"

#include <fstream>
#include <QDebug>
#include <iostream>

#include <json.hpp>

Q_PLUGIN_METADATA(IID "studio.manivault.TaxonomyLoaderPlugin")

using namespace mv;
using namespace mv::gui;

using json = nlohmann::json;

namespace
{
    QString jsonToString(const json& element)
    {
        return QString::fromStdString(element.get<std::string>());
    }

    bool contains(const json& element, const char* attr)
    {
        return element.find(attr) != element.end();
    }

    bool loadAnnotation(const json& element, Annotation& annotation)
    {
        // Check if json annotation contains required fields
        if (!contains(element, "labelset") || !contains(element, "cell_label"))
        {
            qWarning() << "[SCHEMA ERROR] Taxonomy contains annotation without label_set or cell_label field. Skipping annotation..";
            return false;
        }

        // Setting required fields
        annotation.labelset = jsonToString(element["labelset"]);
        annotation.cell_label = jsonToString(element["cell_label"]);
        qDebug() << jsonToString(element["cell_label"]);

        if (contains(element, "cell_fullname")) annotation.cell_fullname = jsonToString(element["cell_fullname"]);
        if (contains(element, "cell_ontology_term_id")) annotation.cell_ontology_term_id = jsonToString(element["cell_ontology_term_id"]);
        if (contains(element, "cell_ontology_term")) annotation.cell_ontology_term = jsonToString(element["cell_ontology_term"]);
        if (contains(element, "rationale")) annotation.rationale = jsonToString(element["rationale"]);
        if (contains(element, "cell_set_accession")) annotation.cell_set_accession = jsonToString(element["cell_set_accession"]);
        if (contains(element, "parent_cell_set_accession")) annotation.parent_cell_set_accession = jsonToString(element["parent_cell_set_accession"]);

        if (contains(element, "cell_ids"))
        {
            const json& cell_id_element = element["cell_ids"];
            for (int j = 0; j < cell_id_element.size(); j++)
                annotation.cell_ids.push_back(jsonToString(cell_id_element[j]));
        }

        return true;
    }

    void traverseAnnotations(Taxonomy& taxonomy, json element)
    {
        Q_ASSERT(element.is_array());

        taxonomy.annotations.resize(element.size());
        for (int i = 0; i < element.size(); i++)
            loadAnnotation(element[i], taxonomy.annotations[i]);

        qDebug() << "Saved annotations: " << taxonomy.annotations.size();
    }

    bool traverseTaxonomy(Taxonomy& taxonomy, json element)
    {
        Q_ASSERT(element.is_object());

        // Check if json annotation contains required title field
        if (!contains(element, "title"))
        {
            qWarning() << "[SCHEMA ERROR] Taxonomy doesn't have a title field. Aborting loading..";
            return false;
        }
        taxonomy.title = jsonToString(element["title"]);

        // Check if json annotation contains required author name field
        if (!contains(element, "author_name"))
        {
            qWarning() << "[SCHEMA ERROR] Taxonomy doesn't have an author_name field. Aborting loading..";
            return false;
        }
        taxonomy.author_name = jsonToString(element["author_name"]);

        if (contains(element, "description")) taxonomy.description = jsonToString(element["description"]);
        if (contains(element, "author_list")) taxonomy.author_list = jsonToString(element["author_list"]);
        if (contains(element, "author_contact")) taxonomy.author_contact = jsonToString(element["author_contact"]);
        if (contains(element, "orcid")) taxonomy.orcid = jsonToString(element["orcid"]);

        if (contains(element, "cellannotation_schema_version")) taxonomy.cellannotation_schema_version = jsonToString(element["cellannotation_schema_version"]);
        if (contains(element, "cellannotation_timestamp")) taxonomy.cellannotation_timestamp = jsonToString(element["cellannotation_timestamp"]);
        if (contains(element, "cellannotation_version")) taxonomy.cellannotation_version = jsonToString(element["cellannotation_version"]);
        if (contains(element, "cellannotation_url")) taxonomy.cellannotation_url = jsonToString(element["cellannotation_url"]);

        int i = 0;
        for (auto& el : element.items())
        {
            std::cout << "key: " << el.key() << std::endl;// << ", value:" << el.value() << '\n';
            //std::cout << "value: " << el.value() << std::endl;

            if (el.key() == "annotations")
            {
                traverseAnnotations(taxonomy, el.value());
                continue;
            }
            //else if (el.value().is_array())
            //{
            //    std::vector<TaxonomyNode> nodes = traverseJsonArray(taxonomy, el.value());

            //    for (TaxonomyNode& node: nodes)
            //        parent._children.push_back(node);
            //}
            //else if (el.value().is_object())
            //{
            //    std::vector<TaxonomyNode> nodes = traverseJsonObject(taxonomy, el.value());

            //    for (TaxonomyNode& node : nodes)
            //        parent._children.push_back(node);
            //}
        }

        return true;
    }
}

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
    try
    {
        //Q_INIT_RESOURCE(met_loader_resources);

        const QString fileName = AskForFileName(tr("H5ad Files (*.h5ad)"));

        // Don't try to load a file if the dialog was cancelled or the file name is empty
        if (fileName.isNull() || fileName.isEmpty())
            return;

        // Load H5ad
        H5adLoader h5adLoader;
        std::string taxonomyStr;
        h5adLoader.LoadFile(fileName);

        //h5adLoader.LoadTaxonomy(fileName, taxonomyStr);

        //// Load taxonomy
        //qDebug() << "Reading taxonomy annotations from file..";
        ////std::ifstream file(fileName.toStdString());
        //json jsondata = json::parse(taxonomyStr);

        //mv::Dataset<CasTaxonomy> taxonomyData = mv::data().createDataset("CAS Taxonomy Data", "Taxonomy");
        //Taxonomy& taxonomy = taxonomyData->getTaxonomy();
        //traverseTaxonomy(taxonomy, jsondata);

        //taxonomy.print();
    }
    catch (std::exception& e)
    {
        std::cerr << "Loading of taxonomy file has failed for unknown reasons." << std::endl;
    }
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
    return supportedTypes;
}
