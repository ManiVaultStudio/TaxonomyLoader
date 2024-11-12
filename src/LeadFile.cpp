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

    void File::OpenStringDataset(std::string datasetPath, std::vector<std::string>& stringData)
    {
        if (!IsFileOpened())
            throw ("Attempt to open string dataset " + datasetPath + " while file is not opened.");

        // Open the dataset
        H5::DataSet dataset = _file->openDataSet(datasetPath);
        
        // Get the datatype of the dataset (expecting strings)
        H5::StrType datatype = dataset.getStrType();

        if (H5Tis_variable_str(datatype.getId()) <= 0) {
            std::cerr << "Dataset is not variable-length string." << std::endl;
            return;
        }

        // Get the dataspace of the dataset (its dimensions)
        H5::DataSpace dataspace = dataset.getSpace();

        // Get the number of elements in the dataset
        hsize_t numElements = dataspace.getSimpleExtentNpoints();

        //// Prepare to read the data into a vector of std::string
        //stringData.resize(numElements);

        // Define memory space to read the data
        H5::DataSpace memspace(1, &numElements);

        char** rdata = (char**)malloc(numElements * sizeof(char*));

        // Read the dataset into rdata (HDF5 handles memory allocation for variable-length strings)
        //dataset.read(rdata, datatype, memspace, dataspace);
        herr_t status = H5Dread(dataset.getId(), datatype.getId(), H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

        // Convert the C-style strings to std::vector<std::string>
        stringData.reserve(numElements);
        for (hssize_t i = 0; i < numElements; ++i) {
            stringData.push_back(std::string(rdata[i]));
        }

        free(rdata);

        //hid_t datasetId = H5Dopen(GetFileId(), datasetPath.c_str(), H5P_DEFAULT);

        //hid_t datatype_id = H5Dget_type(datasetId);
        //if (H5Tis_variable_str(datatype_id) <= 0) {
        //    std::cerr << "Dataset is not variable-length string." << std::endl;
        //}

        //// Step 4: Get the dataspace and number of elements
        //hid_t dataspace_id = H5Dget_space(datasetId);
        //hssize_t numElements = H5Sget_simple_extent_npoints(dataspace_id);

        //// Step 5: Allocate memory to hold the string data (array of C-strings)
        //char** rdata = (char**)malloc(numElements * sizeof(char*));

        //// Step 6: Read the dataset into rdata (HDF5 handles memory allocation for variable-length strings)
        //herr_t status = H5Dread(datasetId, datatype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
        //if (status < 0) {
        //    std::cerr << "Failed to read dataset." << std::endl;
        //    free(rdata);
        //}

        //// Convert rdata to std::vector<StringType>
        //stringVector.reserve(numElements);
        //for (hssize_t i = 0; i < numElements; ++i)
        //{
        //    stringVector = StringType(rdata[i]);
        //    //string_vector.push_back(QString::fromStdString(std::string(rdata[i])));
        //}
        //std::cout << "STRING VECTOR: " << stringVector.size() << std::endl;
        //// Step 8: Clean up the dynamically allocated strings and free the memory
        //for (hssize_t i = 0; i < numElements; ++i) {
        //    free(rdata[i]);  // Free each individual string
        //}
        //free(rdata);  // Free the array of strings

        //if (stringVector.size() > 0)
        //{
        //    //std::cout << stringVector[0] << std::endl;
        //    //std::cout << stringVector[stringVector.size() - 1] << std::endl;
        //}

        //H5Dclose(datasetId);
    }

    void File::OpenIntegerDataset(std::string datasetPath, std::vector<int>& intVector)
    {
        if (!IsFileOpened())
            throw ("Attempt to open integer dataset " + datasetPath + " while file is not opened.");

        try
        {
            // Open the dataset
            H5::DataSet dataset = _file->openDataSet(datasetPath);

            // Get the dataspace of the dataset to determine its size
            H5::DataSpace dataspace = dataset.getSpace();

            // Get the number of dimensions
            int ndims = dataspace.getSimpleExtentNdims();

            // Get the size of each dimension
            std::vector<hsize_t> dims(ndims);
            dataspace.getSimpleExtentDims(dims.data(), nullptr);

            // Allocate a vector to hold the integer data
            intVector.resize(dims[0]);

            // Read the data into the buffer
            dataset.read(intVector.data(), H5::PredType::NATIVE_INT);

            if (intVector.size() > 0)
            {
                std::cout << "First int data: " << intVector[0] << std::endl;
                std::cout << "Last int data: " << intVector[intVector.size() - 1] << std::endl;
            }
        }
        catch (H5::FileIException& e) {
            std::cerr << "File error: " << e.getDetailMsg() << std::endl;
        }
        catch (H5::DataSetIException& e) {
            std::cerr << "Dataset error: " << e.getDetailMsg() << std::endl;
        }
        catch (H5::DataSpaceIException& e) {
            std::cerr << "Dataspace error: " << e.getDetailMsg() << std::endl;
        }
        catch (H5::Exception& e) {
            std::cerr << "General HDF5 error: " << e.getDetailMsg() << std::endl;
        }

        std::cout << "INT VECTOR: " << intVector.size() << std::endl;
    }

    hid_t File::GetFileId()
    {
        return _file->getId();
    }

    bool File::DatasetExists(std::string datasetPath)
    {
        H5E_BEGIN_TRY
        {
            return H5Lexists(_file->getId(), datasetPath.c_str(), H5P_DEFAULT) > 0;
        }
        H5E_END_TRY;
    }

    bool File::IsXSparse()
    {
        // Check if both the "X/indices" and "X/indptr" datasets exist, if so X is sparse
        if (DatasetExists("/X/indices") && DatasetExists("/X/indptr"))
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
