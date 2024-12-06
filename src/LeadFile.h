#pragma once

#include <vector>
#include <string>
#include <iostream>

typedef int64_t hid_t;

#include "H5Cpp.h"

namespace H5
{
    class H5File;
}

namespace LEAD
{
    enum class ObjectType
    {
        GROUP,
        DATASET,
        NAMED_DATATYPE,
        UNKNOWN
    };

    class TreeNode
    {
    public:
        TreeNode(std::string name, ObjectType type);

        void AddChild(TreeNode node);

        std::string GetName() const { return _name; }
        std::vector<TreeNode>& GetChildren();
        const std::vector<TreeNode>& GetChildren() const;

    private:
        std::string             _name;
        ObjectType              _type;

        std::vector<TreeNode>   _children;
    };

    class ObjectDescriptor
    {
    public:
        ObjectDescriptor(std::string name, ObjectType type);

        std::string GetName() const { return _name; }
        ObjectType GetType() const { return _type; }

    private:
        std::string     _name;
        ObjectType      _type;
    };

    class Dataset : public ObjectDescriptor
    {
    public:
        Dataset(std::string name, ObjectType type, H5T_class_t dataType);

        H5T_class_t GetDataType() { return _dataType; }

    private:
        H5T_class_t _dataType;
    };

    class Hierarchy
    {
    public:

    private:

    };

    std::ostream& operator<<(std::ostream& os, const TreeNode& node);

    class File
    {
    public:
        void Open(std::string fileName);

        void OpenStringDataset(std::string datasetPath, std::vector<std::string>& stringVector);
        void OpenIntegerDataset(std::string datasetPath, std::vector<int>& intVector, std::vector<hsize_t>& dims);
        void OpenFloatDataset(std::string datasetPath, std::vector<float>& floatVector, std::vector<hsize_t>& dims);

        hid_t GetFileId();

        bool DatasetExists(std::string datasetPath);
        bool IsXSparse();

        std::vector<Dataset> GetDatasets();
        TreeNode GetLinkTree();

    private:
        bool IsFileOpened() { return _isOpen; }

        template <class DataType>
        void ReadNumericDataset(std::string datasetPath, const H5::PredType& dataType, std::vector<DataType>& outVector, std::vector<hsize_t>& dims);

    private:
        H5::H5File*     _file;

        bool            _isOpen;
    };
}
