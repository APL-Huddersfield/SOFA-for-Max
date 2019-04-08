//
//  CppSofa.cpp
//  SofaMax2
//
//  Created by Dale Johnson on 28/01/2019.
//  Copyright © 2019 Dale Johnson. All rights reserved.
//

#include "CSofa.hpp"
#include "./SOFA/src/sofa.h"
#include "./SOFA/src/ncDim.h"
#include "./SOFA/src/ncVar.h"
#include <iostream>
#include <vector>

t_sofaConvention csofa_getConvention(sofa::File& sofa) {
    std::string strConvention = sofa.GetSOFAConventions();

    if(strConvention == "GeneralFIR") {
        return SOFA_GENERAL_FIR;
    }
    else if(strConvention == "SimpleFreeFieldHRIR") {
        return SOFA_SIMPLE_FREE_FIELD_HRIR;
    }
    else if(strConvention == "GeneralFIRE") {
        return SOFA_GENERAL_FIRE;
    }
    else if(strConvention == "SingleRoomDRIR") {
        return SOFA_SINGLE_ROOM_DRIR;
    }
    else if(strConvention == "MultiSpeakerBRIR") {
        return SOFA_MULTISPEAKER_BRIR;
    }
    return SOFA_UNKNOWN_TYPE;
}

uint64_t csofa_getNumAttributes(const sofa::File& sofa) {
    std::vector<std::string> attributeNames;
    sofa.GetAllAttributesNames(attributeNames);
    return attributeNames.size();
}

uint64_t csofa_getMaxAttributeNameSize(const sofa::File& sofa) {
    uint64_t l = 0;
    std::vector<std::string> attributeNames;
    std::vector<std::string>::iterator iter;
    
    sofa.GetAllAttributesNames(attributeNames);
    /*iter = std::max_element(attributeNames.begin(), attributeNames.end());
    auto i = std::distance(attributeNames.begin(), iter);
    return attributeNames[i].size();*/
    for(auto i = 0; i < attributeNames.size(); ++i) {
        l = attributeNames[i].size() > l ? attributeNames[i].size() : l;
    }
    return l;
}

uint64_t csofa_getMaxAttributeSize(const sofa::File& sofa) {
    std::vector<std::string> attributeNames;
    std::string attribute;
    sofa.GetAllAttributesNames(attributeNames);
    uint64_t maxSize = 0;
    
    for(auto i = 0; i < attributeNames.size(); ++i) {
        attribute = sofa.GetAttributeValueAsString(attributeNames[i]);
        maxSize = attribute.size() > maxSize ? attribute.size() : maxSize;
    }
    
    return maxSize;
}

void csofa_getAttributes(t_sofa* s, t_sofaAttributes* a, const sofa::File& sofa) {
    
    uint64_t numAttributes = csofa_getNumAttributes(sofa);
    uint64_t maxAttributeNameLength = csofa_getMaxAttributeNameSize(sofa);
    uint64_t maxAttributeSize = csofa_getMaxAttributeSize(sofa);
    
    a->names = new char*[numAttributes];
    a->values = new char*[numAttributes];
    a->nameSizes = new uint64_t[numAttributes];
    a->valueSizes = new uint64_t[numAttributes];
    a->numAttributes = numAttributes;
    a->maxAttributeNameSize= maxAttributeNameLength;
    a->maxAttributeSize = maxAttributeSize;
    
    for(uint64_t i = 0; i < numAttributes; ++i) {
        a->names[i] = new char[maxAttributeNameLength + 1]; // +1 allows for null-termination
        a->values[i] = new char[maxAttributeSize + 1];
    }
    
    std::vector<std::string> attributeNames;
    std::string attribute;
    sofa.GetAllAttributesNames(attributeNames);
    for(auto i = 0; i < attributeNames.size(); ++i) {
        attribute = sofa.GetAttributeValueAsString(attributeNames[i]);
        memcpy(a->names[i], attributeNames[i].c_str(), sizeof(char) * attributeNames[i].size());
        memcpy(a->values[i], attribute.c_str(), sizeof(char) * attribute.size());
        a->names[i][attributeNames[i].size()] = '\0';
        a->values[i][attribute.size()] = '\0';
        a->nameSizes[i] = attributeNames[i].size();
        a->valueSizes[i] = attribute.size();
    }
}

t_sofa csofa_openFile(char* filename) {
    t_sofa sofa;
    std::string strFileName(filename);
    std::vector<double> dataIR;

    sofa::File file(strFileName);

    sofa.convention = csofa_getConvention(file);
    sofa.I = 1;
    sofa.M = C_SOFA_FILE(&file)->GetNumMeasurements();
    sofa.R = C_SOFA_FILE(&file)->GetNumReceivers();
    sofa.E = C_SOFA_FILE(&file)->GetNumEmitters();
    sofa.N = C_SOFA_FILE(&file)->GetNumDataSamples();
    sofa.S = C_SOFA_FILE(&file)->GetDimension("S");
    sofa.C = 3;
    sofa.sampleRate = 0.0;
    sofa.numBlocks = 0;

    long dataIRSize;
    sofa.dataIR = nullptr;
    switch(sofa.convention) {
        case SOFA_GENERAL_FIR:
            C_SOFA_GENERAL_FIR(&file)->GetDataIR(dataIR);
            sofa.numBlocks = sofa.M * sofa.R;
            C_SOFA_GENERAL_FIR(&file)->GetSamplingRate(sofa.sampleRate);
            dataIRSize = sofa.numBlocks * sofa.N;
            sofa.dataIR = new double[dataIRSize];
            memcpy(sofa.dataIR, dataIR.data(), sizeof(double) * dataIRSize);
            break;
        case SOFA_SIMPLE_FREE_FIELD_HRIR:
            C_SOFA_SIMPLE_FF_HRIR(&file)->GetDataIR(dataIR);
            sofa.numBlocks = sofa.M * sofa.R;
            C_SOFA_SIMPLE_FF_HRIR(&file)->GetSamplingRate(sofa.sampleRate);
            dataIRSize = sofa.numBlocks * sofa.N;
            sofa.dataIR = new double[dataIRSize];
            memcpy(sofa.dataIR, dataIR.data(), sizeof(double) * dataIRSize);
            break;
        case SOFA_GENERAL_FIRE:
            C_SOFA_GENERAL_FIRE(&file)->GetDataIR(dataIR);
            sofa.numBlocks = sofa.M * sofa.R * sofa.E;
            C_SOFA_GENERAL_FIRE(&file)->GetSamplingRate(sofa.sampleRate);
            dataIRSize = sofa.numBlocks * sofa.N;
            sofa.dataIR = new double[dataIRSize];
            memcpy(sofa.dataIR, dataIR.data(), sizeof(double) * dataIRSize);
            break;
        case SOFA_SINGLE_ROOM_DRIR:
            C_SOFA_SINGLE_ROOM_DRIR(&file)->GetDataIR(dataIR);
            sofa.numBlocks = sofa.M * sofa.R;
            C_SOFA_SINGLE_ROOM_DRIR(&file)->GetSamplingRate(sofa.sampleRate);
            dataIRSize = sofa.numBlocks * sofa.N;
            sofa.dataIR = new double[dataIRSize];
            memcpy(sofa.dataIR, dataIR.data(), sizeof(double) * dataIRSize);
            break;
        case SOFA_MULTISPEAKER_BRIR:
            C_SOFA_MULTISPEAKER_BRIR(&file)->GetDataIR(dataIR);
            sofa.numBlocks = sofa.M * sofa.R * sofa.E;
            C_SOFA_MULTISPEAKER_BRIR(&file)->GetSamplingRate(sofa.sampleRate);
            dataIRSize = sofa.numBlocks * sofa.N;
            sofa.dataIR = new double[dataIRSize];
            memcpy(sofa.dataIR, dataIR.data(), sizeof(double) * dataIRSize);
            break;
        default:
            sofa.dataIR = nullptr;
    }

    // Extract variables
    
    // Positions
    std::vector<double> positions;
    
    C_SOFA_FILE(&file)->GetListenerPosition(positions);
    if(positions.size()) {
        sofa.listenerPoints = new t_point[positions.size()];
        sofa.numListenerPoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.listenerPoints[i].ID = i;
        sofa.listenerPoints[i].pos[0] = positions[i * 3];
        sofa.listenerPoints[i].pos[1] = positions[i * 3 + 1];
        sofa.listenerPoints[i].pos[2] = positions[i * 3 + 2];
    }

    C_SOFA_FILE(&file)->GetReceiverPosition(positions);
    if(positions.size()) {
        sofa.receiverPoints = new t_point[positions.size()];
        sofa.numReceiverPoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.receiverPoints[i].ID = i;
        sofa.receiverPoints[i].pos[0] = positions[i * 3];
        sofa.receiverPoints[i].pos[1] = positions[i * 3 + 1];
        sofa.receiverPoints[i].pos[2] = positions[i * 3 + 2];
    }

    C_SOFA_FILE(&file)->GetSourcePosition(positions);
    if(positions.size()) {
        sofa.sourcePoints = new t_point[positions.size()];
        sofa.numSourcePoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.sourcePoints[i].ID = i;
        sofa.sourcePoints[i].pos[0] = positions[i * 3];
        sofa.sourcePoints[i].pos[1] = positions[i * 3 + 1];
        sofa.sourcePoints[i].pos[2] = positions[i * 3 + 2];
    }

    C_SOFA_FILE(&file)->GetEmitterPosition(positions);
    if(positions.size()) {
        sofa.emitterPoints = new t_point[positions.size()];
        sofa.numEmitterPoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.emitterPoints[i].ID = i;
        sofa.emitterPoints[i].pos[0] = positions[i * 3];
        sofa.emitterPoints[i].pos[1] = positions[i * 3 + 1];
        sofa.emitterPoints[i].pos[2] = positions[i * 3 + 2];
    }
    
    // Views
    std::vector<double> views;
    
    C_SOFA_FILE(&file)->GetListenerView(views);
    if(views.size()) {
        sofa.listenerViews = new t_point[views.size()];
        sofa.numListenerViews = views.size() / 3;
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.listenerViews[i].ID = i;
        sofa.listenerViews[i].pos[0] = views[i * 3];
        sofa.listenerViews[i].pos[1] = views[i * 3 + 1];
        sofa.listenerViews[i].pos[2] = views[i * 3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetSourceView(views);
    if(views.size()) {
        sofa.sourceViews = new t_point[views.size()];
        sofa.numSourceViews = views.size() / 3;
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.sourceViews[i].ID = i;
        sofa.sourceViews[i].pos[0] = views[i * 3];
        sofa.sourceViews[i].pos[1] = views[i * 3 + 1];
        sofa.sourceViews[i].pos[2] = views[i * 3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetReceiverView(views);
    if(views.size()) {
        sofa.receiverViews = new t_point[views.size()];
        sofa.numReceiverViews = views.size() / 3;
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.receiverViews[i].ID = i;
        sofa.receiverViews[i].pos[0] = views[i * 3];
        sofa.receiverViews[i].pos[1] = views[i * 3 + 1];
        sofa.receiverViews[i].pos[2] = views[i * 3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetEmitterView(views);
    if(views.size()) {
        sofa.emitterViews = new t_point[views.size()];
        sofa.numEmitterViews = views.size() / 3;
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.emitterViews[i].ID = i;
        sofa.emitterViews[i].pos[0] = views[i * 3];
        sofa.emitterViews[i].pos[1] = views[i * 3 + 1];
        sofa.emitterViews[i].pos[2] = views[i * 3 + 2];
    }
    
    // Ups
    std::vector<double> ups;
    
    C_SOFA_FILE(&file)->GetListenerUp(ups);
    if(ups.size()) {
        sofa.listenerUps = new t_point[ups.size()];
        sofa.numListenerUps = ups.size() / 3;
    }
    for(auto i = 0; i < ups.size() / 3; ++i) {
        sofa.listenerUps[i].ID = i;
        sofa.listenerUps[i].pos[0] = ups[i * 3];
        sofa.listenerUps[i].pos[1] = ups[i * 3 + 1];
        sofa.listenerUps[i].pos[2] = ups[i * 3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetSourceUp(ups);
    if(ups.size()) {
        sofa.sourceUps = new t_point[ups.size()];
        sofa.numSourceUps = ups.size() / 3;
    }
    for(auto i = 0; i < ups.size() / 3; ++i) {
        sofa.sourceUps[i].ID = i;
        sofa.sourceUps[i].pos[0] = ups[i * 3];
        sofa.sourceUps[i].pos[1] = ups[i * 3 + 1];
        sofa.sourceUps[i].pos[2] = ups[i * 3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetReceiverUp(ups);
    if(ups.size()) {
        sofa.receiverUps = new t_point[ups.size()];
        sofa.numReceiverUps = ups.size() / 3;
    }
    for(auto i = 0; i < ups.size() / 3; ++i) {
        sofa.receiverUps[i].ID = i;
        sofa.receiverUps[i].pos[0] = ups[i * 3];
        sofa.receiverUps[i].pos[1] = ups[i * 3 + 1];
        sofa.receiverUps[i].pos[2] = ups[i * 3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetEmitterUp(ups);
    if(ups.size()) {
        sofa.emitterUps = new t_point[ups.size()];
        sofa.numEmitterUps = ups.size() / 3;
    }
    for(auto i = 0; i < ups.size() / 3; ++i) {
        sofa.emitterUps[i].ID = i;
        sofa.emitterUps[i].pos[0] = ups[i * 3];
        sofa.emitterUps[i].pos[1] = ups[i * 3 + 1];
        sofa.emitterUps[i].pos[2] = ups[i * 3 + 2];
    }
    
    csofa_getAttributes(&sofa, &sofa.attr, file);

    return sofa;
}

t_sofa csofa_newSofa(t_sofaConvention convention, long M, long R, long E, long N, double sampleRate) {
    t_sofa sofa;
    sofa.convention = convention;
    sofa.I = 1;
    sofa.C = 3;
    sofa.M = M;
    sofa.R = R;
    sofa.E = E;
    sofa.N = N;
    sofa.S = 0;
    
    sofa.numListenerPoints = 0;
    sofa.numReceiverPoints = 0;
    sofa.numSourcePoints = 0;
    sofa.numEmitterPoints = 0;
    
    sofa.numListenerViews = 0;
    sofa.numReceiverViews = 0;
    sofa.numSourceViews = 0;
    sofa.numEmitterViews = 0;
    
    sofa.numListenerUps = 0;
    sofa.numReceiverUps = 0;
    sofa.numSourceUps = 0;
    sofa.numEmitterUps = 0;
    
    long dataIRSize = M * R * E * N;
    sofa.dataIR = new double[dataIRSize];
    for(auto i = 0; i < dataIRSize; ++i) {
        sofa.dataIR[i] = 0.0;
    }
    
    sofa.sampleRate = sampleRate;
    sofa.numBlocks = M * R * E;
    
    return sofa;
}

void csofa_destroySofa(t_sofa* sofaFile) {
    if(sofaFile->dataIR) {
        delete[] (sofaFile->dataIR);
    }
    
    if(sofaFile->listenerPoints) {
        delete[] sofaFile->listenerPoints;
    }
    if(sofaFile->receiverPoints) {
        delete[] sofaFile->receiverPoints;
    }
    if(sofaFile->sourcePoints) {
        delete[] sofaFile->sourcePoints;
    }
    if(sofaFile->emitterPoints) {
        delete[] sofaFile->emitterPoints;
    }
    
    if(sofaFile->listenerViews) {
        delete[] sofaFile->listenerViews;
    }
    if(sofaFile->receiverViews) {
        delete[] sofaFile->receiverViews;
    }
    if(sofaFile->sourceViews) {
        delete[] sofaFile->sourceViews;
    }
    if(sofaFile->emitterViews) {
        delete[] sofaFile->emitterViews;
    }
    
    if(sofaFile->listenerUps) {
        delete[] sofaFile->listenerUps;
    }
    if(sofaFile->receiverUps) {
        delete[] sofaFile->receiverUps;
    }
    if(sofaFile->sourceUps) {
        delete[] sofaFile->sourceUps;
    }
    if(sofaFile->emitterUps) {
        delete[] sofaFile->emitterUps;
    }
    
    csofa_clearAttributes(&sofaFile->attr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void csofa_setSampleRate(const t_sofa* sofa, const netCDF::NcFile& file) {
    const std::string varName = "Data.SamplingRate";
    const std::string typeName = "double";
    const std::string dimName = "I";
    const netCDF::NcVar var = file.addVar(varName, typeName, dimName);
    var.putVar(&sofa->sampleRate);
    var.putAtt("Units", "hertz");
}

void csofa_setDataIR(const t_sofa* sofa, const netCDF::NcFile& file) {
    const std::string varName = "Data.IR";
    const std::string typeName = "double";
    std::vector<std::string> dimNames;
    dimNames.push_back("M");
    dimNames.push_back("R");
    
    if(sofa->convention == SOFA_GENERAL_FIRE ||
       sofa->convention == SOFA_MULTISPEAKER_BRIR) {
        dimNames.push_back("E");
    }
    
    dimNames.push_back("N");
    
    const netCDF::NcVar var = file.addVar(varName, typeName, dimNames);
    var.putVar(sofa->dataIR);
}

void csofa_setListenerValues(const t_sofa* sofa, const netCDF::NcFile& file) {
    {
        const std::string varName  = "ListenerPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = file.addVar( varName, typeName, dimNames );
        
        var.putAtt("Type", "cartesian");
        var.putAtt("Units", "metre");
        std::vector<double> listenerPositions;
        for(auto i = 0; i < sofa->numListenerPoints; ++i) {
            for(auto j = 0; j < 3; ++j) {
                listenerPositions.push_back(sofa->listenerPoints[i].pos[j]);
            }
        }
        var.putVar(listenerPositions.data());
    }
    
    {
        const std::string varName  = "ListenerView";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = file.addVar(varName, typeName, dimNames);
        
        var.putAtt( "Type", "cartesian");
        var.putAtt( "Units", "metre");
        std::vector<double> listenerViews;
        for(auto i = 0; i < sofa->numListenerViews; ++i) {
            for(auto j = 0; j < 3; ++j) {
                listenerViews.push_back(sofa->listenerViews[i].pos[j]);
            }
        }
    }
}

void csofa_setReceiverValues(const t_sofa* sofa, const netCDF::NcFile& file) {
    const std::string varName  = "ReceiverPosition";
    const std::string typeName = "double";
    
    std::vector< std::string > dimNames;
    dimNames.push_back("R");
    dimNames.push_back("C");
    dimNames.push_back("I");
    
    const netCDF::NcVar var = file.addVar( varName, typeName, dimNames );
    
    var.putAtt("Type", "cartesian");
    var.putAtt("Units", "metre");
    std::vector<double> receiverPositions;
    for(auto i = 0; i < sofa->numReceiverPoints; ++i) {
        for(auto j = 0; j < 3; ++j) {
            receiverPositions.push_back(sofa->receiverPoints[i].pos[j]);
        }
    }
    var.putVar(receiverPositions.data());
}

void csofa_setEmitterValues(const t_sofa* sofa, const netCDF::NcFile& file) {
    {
        const std::string varName  = "EmitterPosition";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = file.addVar( varName, typeName, dimNames );
        
        var.putAtt("Type", "cartesian");
        var.putAtt("Units", "metre");
        std::vector<double> emitterPositions;
        for(auto i = 0; i < sofa->numEmitterPoints; ++i) {
            for(auto j = 0; j < 3; ++j) {
                emitterPositions.push_back(sofa->emitterPoints[i].pos[j]);
            }
        }
        var.putVar(emitterPositions.data());
    }
    
    {
        const std::string varName  = "EmitterView";
        const std::string typeName = "double";
        
        std::vector< std::string > dimNames;
        dimNames.push_back("I");
        dimNames.push_back("C");
        
        const netCDF::NcVar var = file.addVar(varName, typeName, dimNames);
        
        var.putAtt( "Type", "cartesian");
        var.putAtt( "Units", "metre");
        std::vector<double> emitterViews;
        for(auto i = 0; i < sofa->numEmitterViews; ++i) {
            for(auto j = 0; j < 3; ++j) {
                emitterViews.push_back(sofa->emitterViews[i].pos[j]);
            }
        }
    }
}

void csofa_setSourceValues(const t_sofa* sofa, const netCDF::NcFile& file) {
    const std::string varName  = "SourcePosition";
    const std::string typeName = "double";
    
    std::vector< std::string > dimNames;
    dimNames.push_back("M");
    dimNames.push_back("C");
    
    const netCDF::NcVar var = file.addVar( varName, typeName, dimNames );
    
    if(sofa->convention == SOFA_SIMPLE_FREE_FIELD_HRIR) {
        var.putAtt("Type", "spherical");
        var.putAtt("Units", "degree, degree, metre");
    }
    else {
        var.putAtt("Type", "cartesian");
        var.putAtt("Units", "metre");
    }
    
    std::vector<double> sourcePositions;
    for(auto i = 0; i < sofa->numSourcePoints; ++i) {
        for(auto j = 0; j < 3; ++j) {
            sourcePositions.push_back(sofa->sourcePoints[i].pos[j]);
        }
    }
    var.putVar(sourcePositions.data());
}

t_sofaWriteErr csofa_writeFile(const t_sofa* s, const char* filename) {
    // Preliminary attribute sanity check
    if(!csofa_hasRequiredAttributes(s)) {
        return t_sofaWriteErr::MISSING_ATTR_ERROR;
    }
    
    const netCDF::NcFile::FileMode mode = netCDF::NcFile::newFile;
    const netCDF::NcFile::FileFormat format = netCDF::NcFile::nc4;
    const std::string filePath(filename);
    const netCDF::NcFile outFile(filePath, mode, format);
    
    sofa::Attributes attributes;
    attributes.ResetToDefault();
    
    outFile.addDim("C", 3);
    outFile.addDim("I", 1);
    outFile.addDim("M", s->M);
    outFile.addDim("R", s->R);
    outFile.addDim("E", s->E);
    outFile.addDim("N", s->N);
    
    // Get and write attributes
    auto getAttributePos = [=](const std::string& attrName) {
        for(auto i = 0; i < s->attr.numAttributes; ++i) {
            const std::string testAttrName = std::string(s->attr.names[i]);
            if(attrName.compare(testAttrName) == 0) {
                return i;
            }
        }
        return -1;
    };
    
    for(auto i = 0; i < sofa::Attributes::kNumAttributes; ++i) {
        const sofa::Attributes::Type attrType = static_cast<sofa::Attributes::Type>(i);
        const std::string attrName  = sofa::Attributes::GetName(attrType);
        
        int attrPos = getAttributePos(attrName);
        if(attrPos == -1) {
            continue;
        }
        
        const std::string attrValue = std::string(s->attr.values[attrPos]);
        /*if(attrValue.empty()) {
            continue;
        }*/
        
        outFile.putAtt(attrName, attrValue);
    }
    
    csofa_setSampleRate(s, outFile);
    csofa_setDataIR(s, outFile);
    csofa_setListenerValues(s, outFile);
    csofa_setSourceValues(s, outFile);
    
    return t_sofaWriteErr::NO_WRITE_ERROR;
}

bool csofa_hasRequiredAttributes(const t_sofa* s) {
    auto getAttributePos = [=](const std::string& attrName) {
        for(auto i = 0; i < s->attr.numAttributes; ++i) {
            const std::string testAttrName = std::string(s->attr.names[i]);
            if(attrName.compare(testAttrName) == 0) {
                return i;
            }
        }
        return -1;
    };
    
    // Get and check for required attributes, plus optional ones
    for(auto i = 0; i < sofa::Attributes::kNumAttributes; ++i) {
        const sofa::Attributes::Type attType = static_cast<sofa::Attributes::Type>(i);
        const std::string attName  = sofa::Attributes::GetName(attType);
        bool isRequired = sofa::Attributes::IsRequired(attName);
        int attrPos = getAttributePos(attName);
        if(attrPos == -1) {
            if(isRequired) {
                return false;
            }
            else {
                continue;
            }
        }
        
        const std::string attrValue = std::string(s->attr.values[attrPos]);
        /*if(attrValue.empty() && isRequired) {
            return false;
        }*/
    }
    return true;
}

double* csofa_getMRDataIR(t_sofa* s, uint64_t M, uint64_t R) {
    if(s->convention == SOFA_GENERAL_FIR || s->convention == SOFA_SIMPLE_FREE_FIELD_HRIR) {
        return (s->dataIR + (M * s->R * s->N) + (R * s->N));
    }
    return NULL;
}

double* csofa_getDataIR(t_sofa* s, uint64_t i) {
    return s->dataIR + (i * s->N);
}

double* csofa_getSimpleFreeFieldHRIRDataIR(t_sofa* s, uint64_t M, uint64_t R) {
    return csofa_getMRDataIR(s, M, R);
}

double* csofa_getGeneralFIRDataIR(t_sofa* s, uint64_t M, uint64_t R) {
    return csofa_getMRDataIR(s, M, R);
}

double*  csofa_getGeneralFIREDataIR(t_sofa* s, uint64_t M, uint64_t R, uint64_t E) {
    if(s->convention == SOFA_GENERAL_FIRE) {
        return s->dataIR + (M * s->R * s->E * s->N) + (R * s->E * s->N) + (E * s->N);
    }
    return NULL;
}

double* csofa_getMultiSpeakerBRIR(t_sofa* s, uint64_t M, uint64_t R, uint64_t E) {
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void csofa_newAttributes(t_sofaAttributes* a) {
    
    uint64_t numAttributes = sofa::Attributes::kNumAttributes;
    const uint64_t kMaxValueLength = 512;
    
    std::vector<std::string> names;
    for(auto i = 0; i < numAttributes; ++i) {
        const sofa::Attributes::Type attType = static_cast<sofa::Attributes::Type>(i);
        const std::string name = sofa::Attributes::GetName(attType);
        names.push_back(name);
    }
    
    a->names = new char*[numAttributes];
    a->values = new char*[numAttributes];
    a->nameSizes = new uint64_t[numAttributes];
    a->valueSizes = new uint64_t[numAttributes];
    a->numAttributes = numAttributes;
    a->maxAttributeNameSize = 0;
    
    uint64_t nameSize = 0;
    uint64_t maxNameSize = 0;
    for(auto i = 0; i < numAttributes; ++i) {
        nameSize = names[i].size();
        maxNameSize = nameSize > maxNameSize ? nameSize : maxNameSize;
        
        a->names[i] = new char[nameSize + 1];
        memcpy(a->names, names[i].c_str(), sizeof(char) * nameSize);
        a->names[i][nameSize] = '\0';
        a->nameSizes[i] = nameSize;
        
        a->values[i] = new char[kMaxValueLength];
        memset(a->values[i], '\0', kMaxValueLength);
        a->valueSizes[i] = kMaxValueLength;
    }
    a->maxAttributeNameSize = maxNameSize;
    a->maxAttributeSize = 0; // Set 0 since all characters are null
}

void csofa_clearAttributes(t_sofaAttributes* a) {
    for(auto i = 0; i < a->numAttributes; ++i) {
        delete[] a->names[i];
        delete[] a->values[i];
    }
    delete[] a->names;
    delete[] a->values;
    delete[] a->nameSizes;
    delete[] a->valueSizes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

t_sofaSetDataErr csofa_setRawDataBlock(t_sofa* s, uint64_t i, double* dataBlock, long N) {
    if(dataBlock == NULL) {
        return t_sofaSetDataErr::SET_DATA_INPUT_NULL_ERROR;
    }
    if(s == NULL) {
        return t_sofaSetDataErr::SET_DATA_SOFA_NULL_ERROR;
    }
    if(s->dataIR == NULL) {
        return t_sofaSetDataErr::SET_DATA_SOFA_DATA_NULL_ERROR;
    }
    if(N > s->N) {
        return t_sofaSetDataErr::SET_DATA_INPUT_TOO_BIG_ERROR;
    }
    if(i >= s->numBlocks) {
        return t_sofaSetDataErr::SET_DATA_BLOCK_OUT_OF_RANGE_ERROR;
    }
    
    memcpy(s->dataIR + (i * s->N * sizeof(double)), dataBlock, sizeof(double) * N);
    
    return t_sofaSetDataErr::SET_DATA_NO_ERROR;
}

t_sofaSetDataErr csofa_setMRDataBlock(t_sofa* s, uint64_t M, uint64_t R, double* dataBlock,
                                      long N) {
    uint64_t i = M * s->R + R;
    return csofa_setRawDataBlock(s, i, dataBlock, N);
}

t_sofaSetDataErr csofa_setMREDataBlock(t_sofa* s, uint64_t M, uint64_t R, uint64_t E,
                                       double* dataBlock, long N) {
    uint64_t i = M * s->R * s->E + R * s->E + E;
    return csofa_setRawDataBlock(s, i, dataBlock, N);
}

bool csofa_setPosition(t_sofa* s, t_sofaVarType varType, uint64_t i, t_point* point) {
    uint64_t iMax = 0;
    t_point* dest;
    
    if(s == NULL) {
        return false;
    }
    if(point == NULL) {
        return false;
    }
    
    switch(varType) {
        case SOFA_VAR_LISTENER:
            iMax = s->numListenerPoints;
            dest = s->listenerPoints;
            break;
        case SOFA_VAR_RECEIVER:
            iMax = s->numReceiverPoints;
            dest = s->receiverPoints;
            break;
        case SOFA_VAR_SOURCE:
            iMax = s->numSourcePoints;
            dest = s->sourcePoints;
            break;
        case SOFA_VAR_EMITTER:
            iMax = s->numEmitterPoints;
            dest = s->emitterPoints;
            break;
        default:
            return false;
    }
    
    if(i >= iMax) {
        return false;
    }
    
    dest[i] = *point;
    return true;
}

bool csofa_setView(t_sofa* s, t_sofaVarType varType, uint64_t i, t_point* view) {
    uint64_t iMax = 0;
    t_point* dest;
    
    if(s == NULL) {
        return false;
    }
    if(view == NULL) {
        return false;
    }
    
    switch(varType) {
        case SOFA_VAR_LISTENER:
            iMax = s->numListenerPoints;
            dest = s->listenerPoints;
            break;
        case SOFA_VAR_RECEIVER:
            iMax = s->numReceiverPoints;
            dest = s->receiverPoints;
            break;
        case SOFA_VAR_SOURCE:
            iMax = s->numSourcePoints;
            dest = s->sourcePoints;
            break;
        case SOFA_VAR_EMITTER:
            iMax = s->numEmitterPoints;
            dest = s->emitterPoints;
            break;
        default:
            return false;
    }
    
    if(i >= iMax) {
        return false;
    }
    
    dest[i] = *view;
    return true;
}

bool csofa_setUp(t_sofa* s, t_sofaVarType varType, uint64_t i, t_point* up) {
    uint64_t iMax = 0;
    t_point* dest;
    
    if(s == NULL) {
        return false;
    }
    if(up == NULL) {
        return false;
    }
    
    switch(varType) {
        case SOFA_VAR_LISTENER:
            iMax = s->numListenerPoints;
            dest = s->listenerPoints;
            break;
        case SOFA_VAR_RECEIVER:
            iMax = s->numReceiverPoints;
            dest = s->receiverPoints;
            break;
        case SOFA_VAR_SOURCE:
            iMax = s->numSourcePoints;
            dest = s->sourcePoints;
            break;
        case SOFA_VAR_EMITTER:
            iMax = s->numEmitterPoints;
            dest = s->emitterPoints;
            break;
        default:
            return false;
    }
    
    if(i >= iMax) {
        return false;
    }
    
    dest[i] = *up;
    return true;
}
