#include "LeadFile.h"

#include "H5Cpp.h"

#include <vector>
#include <iostream>

using namespace H5;

namespace
{
    herr_t op_func(hid_t obj, const char* name, const H5O_info2_t* info, void* op_data)
    {
        if (info->type == H5O_TYPE_GROUP)
        {
            auto vec = static_cast<std::vector<std::string>*>(op_data);
            vec->push_back(std::string(name));
        }
        if (info->type == H5O_TYPE_DATASET)
        {

        }

        return 0;
    }

    herr_t op_funcL(hid_t loc_id, const char* name, const H5L_info_t* info, void* op_data)
    {
        herr_t     status;
        H5O_info_t infobuf;
        
        auto rootNode = static_cast<LEAD::TreeNode*>(op_data);

        /*
         * Get type of the object and display its name and type.
         * The name of the object is passed to this function by
         * the Library.
         */
        status = H5Oget_info_by_name(loc_id, name, &infobuf, H5O_INFO_ALL, H5P_DEFAULT);
        
        switch (infobuf.type)
        {
        case H5O_TYPE_GROUP:
            printf("  Group: %s\n", name);

            //LEAD::TreeNode child(name, LEAD::NodeType::GROUP);

            //Group group = _file->openGroup(child.GetName());

            //herr_t status = H5Literate(group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, op_funcL, static_cast<void*>(&child));

            rootNode->AddChild(LEAD::TreeNode(name, LEAD::ObjectType::GROUP));

            break;
        case H5O_TYPE_DATASET:
            printf("  Dataset: %s\n", name);

            rootNode->AddChild(LEAD::TreeNode(name, LEAD::ObjectType::DATASET));
            break;
        case H5O_TYPE_NAMED_DATATYPE:
            printf("  Datatype: %s\n", name);

            rootNode->AddChild(LEAD::TreeNode(name, LEAD::ObjectType::NAMED_DATATYPE));
            break;
        default:
            printf("  Unknown: %s\n", name);

            rootNode->AddChild(LEAD::TreeNode(name, LEAD::ObjectType::UNKNOWN));
        }

        return 0;
    }

    class VisitObject
    {
    public:
        VisitObject(LEAD::File* file) : file(file) {}
        std::vector<LEAD::Dataset> datasets;
        std::vector<LEAD::ObjectDescriptor> groups;
        LEAD::File* file;
    };

    herr_t op_func_visit(hid_t loc_id, const char* name, const H5O_info_t* info, void* op_data)
    {
        printf("/"); /* Print root group in object path */

        auto visitObject = static_cast<VisitObject*>(op_data);

        /*
         * Check if the current object is the root group, and if not print
         * the full path name and type.
         */
        if (name[0] == '.') /* Root group, do not print '.' */
            printf("  (Group)\n");
        else
            switch (info->type) {
            case H5O_TYPE_GROUP:
            {
                printf("%s  (Group)\n", name);

                visitObject->groups.push_back(LEAD::ObjectDescriptor(name, LEAD::ObjectType::GROUP));
                break;
            }
            case H5O_TYPE_DATASET:
            {
                printf("%s  (Dataset)\n", name);

                // Open the data and find out its type
                std::cout << "Trying to open " << name << std::endl;
                hid_t datasetId = H5Dopen(visitObject->file->GetFileId(), name, H5P_DEFAULT);
                std::cout << "Opened " << name << std::endl;
                hid_t datasetTypeId = H5Dget_type(datasetId);
                H5T_class_t datasetClass = H5Tget_class(datasetTypeId);
                H5Dclose(datasetId);
                std::cout << "Closed " << name << std::endl;
                visitObject->datasets.push_back(LEAD::Dataset(name, LEAD::ObjectType::DATASET, datasetClass));
                break;
            }
            case H5O_TYPE_NAMED_DATATYPE:
            {
                printf("%s  (Datatype)\n", name);

                //nodes->push_back(LEAD::ObjectDescriptor(name, LEAD::ObjectType::NAMED_DATATYPE));
                break;
            }
            default:
                printf("%s  (Unknown)\n", name);

                //nodes->push_back(LEAD::ObjectDescriptor(name, LEAD::ObjectType::UNKNOWN));
            }

        return 0;
    }

    /************************************************************

      Operator function for H5Lvisit.  This function simply
      retrieves the info for the object the current link points
      to, and calls the operator function for H5Ovisit.

     ************************************************************/
    herr_t
        op_func_L(hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data)
    {
        herr_t     status;
        H5O_info_t infobuf;

        /*
         * Get type of the object and display its name and type.
         * The name of the object is passed to this function by
         * the Library.
         */
#if H5_VERSION_GE(1, 12, 0) && !defined(H5_USE_110_API) && !defined(H5_USE_18_API) && !defined(H5_USE_16_API)
        status = H5Oget_info_by_name(loc_id, name, &infobuf, H5O_INFO_ALL, H5P_DEFAULT);
#else
        status = H5Oget_info_by_name(loc_id, name, &infobuf, H5P_DEFAULT);
#endif
        return op_func(loc_id, name, &infobuf, operator_data);
    }
}

namespace LEAD
{
    TreeNode::TreeNode(std::string name, ObjectType type) :
        _name(name),
        _type(type)
    {

    }

    void TreeNode::AddChild(TreeNode node)
    {
        std::cout << node.GetName() << "Added" << std::endl;
        _children.push_back(node);
    }

    std::vector<TreeNode>& TreeNode::GetChildren()
    {
        return _children;
    }

    const std::vector<TreeNode>& TreeNode::GetChildren() const
    {
        return _children;
    }

    std::ostream& operator<<(std::ostream& os, const TreeNode& node)
    {
        std::string name = node.GetName();

        os << name << '\n';
        os << "Num children: " << node.GetChildren().size() << '\n';
        for (const TreeNode& child : node.GetChildren())
        {
            os << "\t" << child.GetName() << '\n';

            for (const TreeNode& child1 : child.GetChildren())
            {
                os << "\t\t" << child1.GetName() << '\n';

                for (const TreeNode& child2 : child1.GetChildren())
                {
                    os << "\t\t\t" << child2.GetName() << '\n';
                }
            }
        }

        os << std::endl;

        return os;
    }

    ObjectDescriptor::ObjectDescriptor(std::string name, ObjectType type) :
        _name(name),
        _type(type)
    {

    }

    Dataset::Dataset(std::string name, ObjectType type, H5T_class_t dataType) :
        ObjectDescriptor(name, type),
        _dataType(dataType)
    {

    }

    void File::Open(std::string fileName)
    {
        _file = new H5File(fileName, H5F_ACC_RDONLY);

        _isOpen = true;
    }

    hid_t File::GetFileId()
    {
        return _file->getId();
    }

    bool File::IsXSparse()
    {
        H5E_BEGIN_TRY
        {
            // Try to open "X/indices" and "X/indptr" datasets
            htri_t indicesExists = H5Lexists(_file->getId(), "/X/indices", H5P_DEFAULT);
            htri_t indptrExists = H5Lexists(_file->getId(), "/X/indptr", H5P_DEFAULT);

            if (indicesExists >= 0 && indptrExists >= 0)
            {
                // Both datasets exist, so the matrix is stored in sparse format
                std::cout << "Sparse dataset X found." << std::endl;
                return true;
            }
            else {
                // If we can't open both datasets, it's likely dense
                std::cout << "Dense dataset X found." << std::endl;
                return false;
            }
        }
        H5E_END_TRY;
    }

    std::vector<Dataset> File::GetDatasets()
    {
        if (!_isOpen)
        {
            std::cerr << "File not opened yet, cannot get datasets.";
            return std::vector<Dataset>();
        }

        hid_t currentId = _file->getId();

        VisitObject visitObject(this);

        herr_t status = H5Ovisit(_file->getId(), H5_INDEX_NAME, H5_ITER_NATIVE, op_func_visit, static_cast<void*>(&visitObject), H5O_INFO_ALL);

        for (int i = 0; i < visitObject.datasets.size(); i++)
        {
            std::cout << "Dataset: " << visitObject.datasets[i].GetName() << std::endl;
        }
        for (int i = 0; i < visitObject.groups.size(); i++)
        {
            std::cout << "Group: " << visitObject.groups[i].GetName() << std::endl;
        }

        return visitObject.datasets;
    }

    TreeNode File::GetLinkTree()
    {
        if (!_isOpen)
        {
            std::cerr << "File not opened yet, cannot build link tree.";
            return TreeNode("Unopened file", ObjectType::UNKNOWN);
        }

        hid_t currentId = _file->getId();

        std::vector<ObjectDescriptor> descriptors;

        #if H5_VERSION_GE(1, 12, 0) && !defined(H5_USE_110_API) && !defined(H5_USE_18_API) && !defined(H5_USE_16_API)
            herr_t status = H5Ovisit(_file->getId(), H5_INDEX_NAME, H5_ITER_NATIVE, op_func_visit, static_cast<void*>(&descriptors), H5O_INFO_ALL);
        #else
            status = H5Ovisit(file, H5_INDEX_NAME, H5_ITER_NATIVE, op_func, NULL);
        #endif
        
        for (int i = 0; i < descriptors.size(); i++)
        {
            std::cout << descriptors[i].GetName() << std::endl;
        }

        //TreeNode root(_file->getFileName(), LEAD::NodeType::GROUP);
        //herr_t status = H5Literate(currentId, H5_INDEX_NAME, H5_ITER_INC, NULL, op_funcL, static_cast<void*>(&root));

        //for (TreeNode& child : root.GetChildren())
        //{
        //    Group group = _file->openGroup(child.GetName());

        //    herr_t status = H5Literate(group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, op_funcL, static_cast<void*>(&child));
        //}

        //std::cout << root << std::endl;

        return TreeNode("Unopened file", ObjectType::UNKNOWN);
    }
}
