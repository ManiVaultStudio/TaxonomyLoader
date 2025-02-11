#include "H5adLoader.h"

#include "H5Cpp.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <TextData/TextData.h>

#include <iostream>

using namespace H5;

namespace
{
    bool endsWith(const std::string& fullString, const std::string& ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        else {
            return false;
        }
    }

    std::string replaceAfterLastSlash(const std::string& fullString, const std::string& replacement) {
        size_t lastSlashPos = fullString.find_last_of('/');

        if (lastSlashPos != std::string::npos) {
            return fullString.substr(0, lastSlashPos + 1) + replacement;
        }
        else {
            return fullString;  // If no slash is found, return the original string.
        }
    }

    void attr_op(H5::H5Location& loc, const std::string attr_name,
        void* operator_data) {
        std::cout << attr_name << std::endl;
    }

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

    herr_t
        op_funcL(hid_t loc_id, const char* name, const H5L_info_t* info, void* operator_data)
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
        switch (infobuf.type) {
        case H5O_TYPE_GROUP:
            printf("  Group: %s\n", name);
            break;
        case H5O_TYPE_DATASET:
            printf("  Dataset: %s\n", name);
            break;
        case H5O_TYPE_NAMED_DATATYPE:
            printf("  Datatype: %s\n", name);
            break;
        default:
            printf("  Unknown: %s\n", name);
        }

        return 0;
    }
}

void H5adLoader::LoadTaxonomy(QString fileName, std::string& taxonomyStr)
{
    H5File file(fileName.toStdString(), H5F_ACC_RDONLY);

    std::vector<std::string> groups;
    herr_t status = H5Ovisit(file.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, op_func, static_cast<void*>(&groups), H5O_INFO_ALL);

    bool written = false;
    for (int i = 0; i < groups.size(); i++)
    {
        QString groupName = QString::fromStdString(groups[i]);

        std::cout << i << ": " << groupName.toStdString() << std::endl;
        if (groupName == "uns")
        {
            Group group = file.openGroup(groupName.toStdString());

            H5::DataSet   dataset = file.openDataSet(groupName.toStdString() + "/cas");
            H5::DataSpace dataspace = dataset.getSpace();
            H5::StrType   datatype = dataset.getStrType();

            dataset.read(taxonomyStr, datatype, dataspace);

            std::cout << "CAS:" << taxonomyStr << std::endl;

            dataset.close();
        }
    }

    file.close();
}

// Function to convert CSR to a 1D row-major dense matrix
void csr_to_dense_1d(const std::vector<float>& data,
    const std::vector<int>& indices,
    const std::vector<int>& indptr,
    std::vector<float>& dense_matrix_1d,
    int64_t num_rows, int64_t num_cols) {

    //std::cout << std::vector<float>::max_size() << std::endl;
    // Initialize the 1D dense matrix with zeros (size: num_rows * num_cols)
    std::cout << dense_matrix_1d.max_size() << std::endl;

    dense_matrix_1d.resize(num_rows * num_cols, 0);
    
    std::cout << "Dense matrix rows / cols: " << num_rows << " " << num_cols << std::endl;
    std::cout << "Dense matrix wanted size: " << num_rows * num_cols << std::endl;
    std::cout << "Dense matrix size: " << dense_matrix_1d.size() << std::endl;
    // Iterate through each row
    for (int i = 0; i < num_rows; ++i) {
        int row_start = indptr[i];     // Start of the non-zero elements in the row
        int row_end = indptr[i + 1];   // End of the non-zero elements in the row

        // Fill in the non-zero elements of the dense matrix in 1D format (row-major order)
        for (int j = row_start; j < row_end; ++j) {
            int col_idx = indices[j];  // Get the column index of the non-zero element
            dense_matrix_1d[i * num_cols + col_idx] = data[j];  // Row-major order
        }
    }
}

void H5adLoader::LoadX()
{
    std::cout << "LoadX" << std::endl;
    if (lf.IsXSparse())
    {
        hid_t Xdataset = H5Gopen(lf.GetFileId(), "X", H5P_DEFAULT);
        hid_t XdatasetId = H5Dopen(lf.GetFileId(), "X/data", H5P_DEFAULT);
        hid_t XIndicesDatasetId = H5Dopen(lf.GetFileId(), "X/indices", H5P_DEFAULT);
        hid_t XIndptrDatasetId = H5Dopen(lf.GetFileId(), "X/indptr", H5P_DEFAULT);

        herr_t status;

        try
        {
            if (Xdataset < 0 || XdatasetId < 0 || XIndicesDatasetId < 0 || XIndptrDatasetId < 0)
            {
                throw std::runtime_error("Expected sparse dataset, but it failed to open or does not exist.");
            }

            std::cout << "Sparse dataset opened successfully." << std::endl;

            ///
            hsize_t dims[1];  // Assuming the 'shape' attribute is a 1D array
            int shape[2];     // Array to store the two integers (rows and columns)
                // Open the group 'X'
            hid_t group_id = H5Gopen(lf.GetFileId(), "X", H5P_DEFAULT);

            // Open the attribute 'shape'
            hid_t attr_id = H5Aopen(group_id, "shape", H5P_DEFAULT);

            // Get the attribute dataspace and check the number of elements
            hid_t attr_space = H5Aget_space(attr_id);
            H5Sget_simple_extent_dims(attr_space, dims, NULL);

            // Ensure the attribute has the correct number of elements
            if (dims[0] == 2) {
                // Read the attribute
                H5Aread(attr_id, H5T_NATIVE_INT, shape);
                printf("Shape: rows=%d, columns=%d\n", shape[0], shape[1]);
            }
            else {
                throw std::runtime_error("Expected X.shape attribute to be 2-dimensional, but it isn't");
            }
            ///

            hid_t dataspaceIdX = H5Dget_space(XdatasetId);
            int rank = H5Sget_simple_extent_ndims(dataspaceIdX);
            if (rank != 1)
            {
                throw "Expected a 1D dataset but got " + std::to_string(rank) + "D.";
            }
            
            hsize_t datadim[1];  // Array to hold the sizes of each dimension
            H5Sget_simple_extent_dims(dataspaceIdX, datadim, NULL);
            
            hsize_t numData = datadim[0];
            std::cout << "Number of rows: " << numData << std::endl;

            std::vector<float> data(numData);
            status = H5Dread(XdatasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
            if (status < 0) {
                throw "Failed to read X dataset";
            }

            // Indices dataspace
            hid_t dataspace_id1 = H5Dget_space(XIndicesDatasetId);
            hssize_t num_elements1 = H5Sget_simple_extent_npoints(dataspace_id1);

            // Indptr dataspace
            hid_t dataspace_id2 = H5Dget_space(XIndptrDatasetId);
            hssize_t num_elements2 = H5Sget_simple_extent_npoints(dataspace_id2);

            std::vector<int> indices(num_elements1);
            std::vector<size_t> bigIndices(num_elements1);
            status = H5Dread(XIndicesDatasetId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, indices.data());
            if (status < 0) {
                throw "Failed to read X indices dataset";
            }

            std::transform(indices.begin(), indices.end(), bigIndices.begin(), [](int val) { return static_cast<size_t>(val); });
            indices.clear();

            std::vector<int> indptr(num_elements2);
            std::vector<size_t> bigIndPtr(num_elements2);

            status = H5Dread(XIndptrDatasetId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, indptr.data());
            if (status < 0) {
                throw "Failed to read X indptr dataset";
            }

            std::transform(indptr.begin(), indptr.end(), bigIndPtr.begin(), [](int val) { return static_cast<size_t>(val); });
            indptr.clear();

            std::cout << "Post read4: " << num_elements2 << std::endl;

            X = mv::data().createDataset<Points>("Points", QString::fromStdString("X"));

            Points::Experimental::setSparseData(X.get(), shape[0], shape[1], bigIndPtr, bigIndices, data);
            //std::vector<float> denseMatrix(shape[0], shape[1]);
            //csr_to_dense_1d(data, indices, indptr, denseMatrix, shape[0], shape[1]);

            //X->setData(std::move(denseMatrix), shape[1]);

            //std::vector<float> denseMatrix(shape[0], 0);
            //csr_to_dense_1d(data, indices, indptr, denseMatrix, shape[0], shape[1]);

            //X->setData(std::move(denseMatrix), 1); //shape[1]

            mv::events().notifyDatasetDataChanged(X);
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        H5Dclose(XdatasetId);
        H5Dclose(XIndicesDatasetId);
        H5Dclose(XIndptrDatasetId);
    }
    else
    {
        std::cout << "Trying to open X" << std::endl;
        hid_t XdatasetId = H5Dopen(lf.GetFileId(), "X", H5P_DEFAULT);
        std::cout << "Opened X" << std::endl;
        herr_t status;

        try
        {
            if (XdatasetId < 0)
                throw "Dense X dataset does not exist or failed to open.";

            std::cout << "Dense dataset opened successfully." << std::endl;

            hid_t dataspaceIdX = H5Dget_space(XdatasetId);
            int rank = H5Sget_simple_extent_ndims(dataspaceIdX);
            if (rank != 2)
            {
                throw "Expected a 2D dataset but got " + std::to_string(rank) + "D.";
            }

            hsize_t dims[2];  // Array to hold the sizes of each dimension
            H5Sget_simple_extent_dims(dataspaceIdX, dims, NULL);

            hsize_t numRows = dims[0];
            hsize_t numCols = dims[1];
            std::cout << "Number of rows: " << numRows << std::endl;
            std::cout << "Number of cols: " << numCols << std::endl;

            std::vector<float> data(numRows * numCols);
            status = H5Dread(XdatasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
            if (status < 0) {
                throw "Failed to read X dataset";
            }

            X = mv::data().createDataset<Points>("Points", QString::fromStdString("X"));

            X->setData(data, numCols);

            mv::events().notifyDatasetDataChanged(X);
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }

        H5Dclose(XdatasetId);
    }
}

void H5adLoader::LoadFile(QString fileName)
{
    lf.Open(fileName.toStdString());

    std::vector<LEAD::Dataset> datasets = lf.GetDatasets();
    std::cout << "Got datasets" << std::endl;
    LoadX();

    for (int i = 0; i < datasets.size(); i++)
    {
        LEAD::Dataset& dataset = datasets[i];

        std::cout << "Dataset " << dataset.GetName();

        if (dataset.GetName() == "X" || QString::fromStdString(dataset.GetName()).startsWith("X/"))
        {
            continue;
        }

        if (endsWith(dataset.GetName(), "codes") && lf.DatasetExists(replaceAfterLastSlash(dataset.GetName(), "categories")))
            continue;
        if (endsWith(dataset.GetName(), "categories") && lf.DatasetExists(replaceAfterLastSlash(dataset.GetName(), "codes")))
        {
            // Categorical dataset

            // Categories part
            std::vector<QString> qStringData;

            hid_t datasetId = H5Dopen(lf.GetFileId(), dataset.GetName().c_str(), H5P_DEFAULT);

            hid_t datatype_id = H5Dget_type(datasetId); 
            if (H5Tis_variable_str(datatype_id))
            {
                std::vector<std::string> stringData;
                lf.OpenStringDataset(dataset.GetName().c_str(), stringData);
                if (stringData.size() > 0)
                {
                    std::cout << stringData[0] << std::endl;
                    std::cout << stringData[stringData.size() - 1] << std::endl;
                }

                // Convert the std::strings to QStrings
                qStringData.reserve(stringData.size());
                for (hssize_t i = 0; i < stringData.size(); ++i) {
                    qStringData.push_back(QString::fromStdString(stringData[i]));
                }
            }
            else {
                std::cerr << "Dataset is not variable-length string." << std::endl;
            }

            H5Dclose(datasetId);

            // Codes part
            std::string datasetCodesName = replaceAfterLastSlash(dataset.GetName(), "codes");
            datasetId = H5Dopen(lf.GetFileId(), datasetCodesName.c_str(), H5P_DEFAULT);

            // Step 4: Get the dataspace and number of elements
            hid_t dataspace_id = H5Dget_space(datasetId);
            hssize_t num_elements = H5Sget_simple_extent_npoints(dataspace_id);

            // Step 5: Allocate a vector to hold the integer data
            std::vector<int> int_vector(num_elements);

            // Step 6: Read the dataset directly into the std::vector<int>
            herr_t status = H5Dread(datasetId, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, int_vector.data());
            if (status < 0) {
                return;
            }
            std::cout << "INT VECTOR: " << int_vector.size() << std::endl;

            if (int_vector.size() > 0)
            {
                std::cout << "First int data: " << int_vector[0] << std::endl;
                std::cout << "Last int data: " << int_vector[int_vector.size() - 1] << std::endl;
            }

            H5Dclose(datasetId);

            mv::Dataset<Clusters> clustersData = mv::data().createDataset<Clusters>("Cluster", QString::fromStdString(replaceAfterLastSlash(dataset.GetName(), "")), X);

            for (int i = 0; i < qStringData.size(); i++)
            {
                QString& clusterName = qStringData[i];

                Cluster cluster;
                std::vector<unsigned int> indices;
                for (int c = 0; c < int_vector.size(); c++)
                {
                    if (int_vector[c] == i)
                        indices.push_back(c);
                }
                if (clusterName.isNull() || clusterName.isEmpty())
                    clusterName = QString("");
                cluster.setName(clusterName);
                cluster.setIndices(indices);
                
                clustersData->addCluster(cluster);
            }

            Cluster::colorizeClusters(clustersData->getClusters());

            mv::events().notifyDatasetDataChanged(clustersData);

            continue;
        }

        switch (dataset.GetDataType())
        {
        case H5T_INTEGER:
        {
            std::cout << " is of type INTEGER.";

            std::vector<int> intVector;
            std::vector<hsize_t> dims;
            lf.OpenIntegerDataset(dataset.GetName(), intVector, dims);

            mv::Dataset<Points> pointData = mv::data().createDerivedDataset<Points>(QString::fromStdString(dataset.GetName()), X);
            pointData->setData(intVector, dims.size());
            mv::events().notifyDatasetDataChanged(pointData);

            break;
        }
        case H5T_FLOAT:
        {
            std::cout << " is of type FLOAT.";

            std::vector<float> floatVector;
            std::vector<hsize_t> dims;
            lf.OpenFloatDataset(dataset.GetName(), floatVector, dims);
            
            mv::Dataset<Points> pointData = mv::data().createDerivedDataset<Points>(QString::fromStdString(dataset.GetName()), X);
            pointData->setData(floatVector, dims.size());
            mv::events().notifyDatasetDataChanged(pointData);

            //hid_t datasetId = H5Dopen(lf.GetFileId(), dataset.GetName().c_str(), H5P_DEFAULT);

            //// Step 3: Get the dataspace of the dataset
            //hid_t dataspace_id = H5Dget_space(datasetId);

            //// Step 4: Get the number of dimensions and their sizes
            //int rank = H5Sget_simple_extent_ndims(dataspace_id);  // Get the number of dimensions
            //if (rank == 1)
            //{
            //    // Sparse data?
            //}
            //else if (rank == 2)
            //{
            //}
            //else
            //{
            //    std::cerr << "Expected a 2D dataset but got " << rank << "D." << std::endl;
            //    H5Sclose(dataspace_id);
            //    H5Dclose(datasetId);
            //    return;
            //}

            //// Step 5: Get the dimensions (rows and columns)
            //hsize_t dims[2];  // Array to hold the sizes of each dimension
            //H5Sget_simple_extent_dims(dataspace_id, dims, NULL);

            //// dims[0] is the number of rows, dims[1] is the number of columns
            //hsize_t num_rows = dims[0];
            //hsize_t num_cols = rank == 2 ? dims[1] : 1;

            //// Step 6: Print the number of rows and columns
            //std::cout << "Number of rows: " << num_rows << std::endl;
            //std::cout << "Number of columns: " << num_cols << std::endl;

            break;
        }
        case H5T_STRING:
        {
            std::cout << " is of type STRING.";

            std::vector<std::string> stringData;
            lf.OpenStringDataset(dataset.GetName(), stringData);

            // Convert the std::strings to QStrings
            std::vector<QString> qStringData;
            qStringData.reserve(stringData.size());
            for (hssize_t i = 0; i < stringData.size(); ++i) {
                qStringData.push_back(QString::fromStdString(stringData[i]));
            }

            if (qStringData.size() > 0)
            {
                std::cout << qStringData[0].toStdString() << std::endl;
                std::cout << qStringData[qStringData.size()-1].toStdString() << std::endl;
            }

            mv::Dataset<Text> textData = mv::data().createDataset<Text>("Text", QString::fromStdString(dataset.GetName()), X);

            textData->addColumn("test", qStringData);
            mv::events().notifyDatasetDataChanged(textData);

            break;
        }
        case H5T_COMPOUND:
            std::cout << " is of type COMPOUND.";
            break;
        case H5T_ENUM:
            std::cout << " is of type ENUM.";
            break;
        case H5T_ARRAY:
            std::cout << " is of type ARRAY.";
            break;
        case H5T_VLEN:
            std::cout << " is of type VARIABLE LENGTH.";
            break;
        case H5T_OPAQUE:
            std::cout << " is of type OPAQUE.";
            break;
        case H5T_REFERENCE:
            std::cout << " is of type REFERENCE.";
            break;
        case H5T_BITFIELD:
            std::cout << " is of type BITFIELD.";
            break;
        default:
            std::cout << " is of unsupported datatype.";
            break;
        }
        std::cout << std::endl;
    }

    return;
    H5File file(fileName.toStdString(), H5F_ACC_RDONLY);

    std::vector<std::string> groups;
    herr_t status = H5Ovisit(file.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, op_func, static_cast<void*>(&groups), H5O_INFO_ALL);

    //std::vector<std::string> group_names;
    //H5Literate(file.getId(), H5_INDEX_NAME, H5_ITER_INC, nullptr, file_info, &group_names);

    for (int i = 0; i < groups.size(); i++)
    {
        std::cout << "Opening group: " << groups[i] << std::endl;
        H5::Group group = file.openGroup(groups[i]);

        //H5Literate()
        herr_t status = H5Literate(group.getId(), H5_INDEX_NAME, H5_ITER_NATIVE, NULL, op_funcL, NULL);

        group.close();
    }

    bool written = false;
    for (int i = 0; i < groups.size(); i++)
    {
        //H5::Group group = file.openGroup(groups[i]);
        //
        QString groupName = QString::fromStdString(groups[i]);

        //try
        //{
        //    H5::DataSet dataset = file.openDataSet(groupName);
        //}
        //catch (H5::LocationException e)
        //{
        //    return false;
        //}

        //H5::DataSpace data_space = data_set.getSpace();
        //H5::DataType data_type = data_set.getDataType();

        std::cout << i << ": " << groupName.toStdString() << std::endl;
        if (groupName == "uns")
        {
            Group group = file.openGroup(groupName.toStdString());
            
            H5::DataSet   dataset = file.openDataSet(groupName.toStdString() + "/cas");
            H5::DataSpace dataspace = dataset.getSpace();
            H5::StrType   datatype = dataset.getStrType();

            // allocate output
            std::string data;

            // read output
            dataset.read(data, datatype, dataspace);

            //StrType casData = file.openStrType(groupName.toStdString() + "/cas");
            //
            //std::cout << "Dataset obj name: " << casData.getObjName() << std::endl;

            //std::string field_value;
            //casData.read(field_value, datatype, dataspace);
            //
            //std::cout << "CAS:" << data << std::endl;

            dataset.close();

            //Attribute attribute = group.openAttribute("cas");

            //std::cout << "Attribute obj name: " << attribute.getName() << std::endl;

            //StrType type = attribute.getStrType();

            //// Read the data
            //std::string casText;
            //attribute.read(type, casText);

            //std::cout << "CAS: " << casText << std::endl;
            ////ParseComments(comments, recording);

            //attribute.close();

            //Recording recording;
            //ReadTimeseries(file, group.toStdString(), recording);

            //experiment.addAcquisition(recording);
        }
        if (groupName.startsWith("stimulus/presentation/"))
        {
            //Recording recording;
            //ReadTimeseries(file, group.toStdString(), recording);

            //experiment.addStimulus(recording);
        }
    }

    file.close();

    //for (int i = 0; i < experiment.getAcquisitions()[0].comments.size(); i++)
    //{
    //    std::cout << "Comment: " << experiment.getAcquisitions()[0].comments[i] << std::endl;
    //}

    //dataset.iterateAttrs((H5::attr_operator_t) attr_op);
}
