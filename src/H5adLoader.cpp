#include "H5adLoader.h"

#include "H5Cpp.h"

#include <iostream>

using namespace H5;

namespace
{
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

void H5adLoader::LoadFile(QString fileName)
{
    //Experiment experiment;

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
            std::cout << "CAS:" << data << std::endl;

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
