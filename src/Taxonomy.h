#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <QString>
#include <QVector>
#include <QMap>

enum class Verdict
{
    AGRRE, DISAGREE
};

class Review
{
    QString datestamp;
    QString reviewer;
    Verdict verdict;
    QString explanation;
};

class TransferredAnnotation
{
public:
    QString transferred_cell_label;
    QString source_taxonomy;
    QString source_node_accession;
    QString algorithm_name;
    QString comment;
};

class Cell
{
public:
    QString cell_id;
    float confidence;
    // author_categories, what type?
};

class Annotation
{
public:
    /** REQUIRED: The unique name of the set of cell annotations. */
    QString labelset;
    /** REQUIRED:  This denotes any free-text term which the author uses to annotate cells,
                   i.e. the preferred cell label name used by the author. */
    QString cell_label;

    // Free-text fields
    QString cell_fullname;
    QString cell_ontology_term_id;
    QString cell_ontology_term;
    QString rationale;
    QString cell_set_accession;
    QString parent_cell_set_accession;

    // Free text arrays
    QVector<QString> cell_ids;
    QVector<QString> rationale_dois;
    QVector<QString> marker_gene_evidence;
    QVector<QString> synonyms;
    QVector<QString> negative_marker_gene_evidence;

    QVector<Review> reviews;
    QMap<QString, QString> author_annotation_fields;
    QVector<TransferredAnnotation> transferred_annotations;
    QVector<Cell> cells;
};

class Taxonomy
{
public:
    static Taxonomy fromJsonFile();

    void print();

    /** REQUIRED: The title of the dataset. */
    QString title;
    /** REQUIRED: Primary author's name. */
    QString author_name;

    // Free text fields
    /** The description of the dataset. */
    QString description;

    /** This field stores a list of users who are included in the project as collaborators, regardless of their specific role. */
    QString author_list;
    /** Primary author's contact. */
    QString author_contact;
    /** Primary author's orcid. */
    QString orcid;

    /** The schema version, the cell annotation open standard. */
    QString cellannotation_schema_version;
    /** The timestamp of all cell annotations published (per dataset). */
    QString cellannotation_timestamp;
    /** The version for all cell annotations published (per dataset). */
    QString cellannotation_version;
    /** A persistent URL of all cell annotations published (per dataset). */
    QString cellannotation_url;

    std::vector<Annotation> annotations;
};
