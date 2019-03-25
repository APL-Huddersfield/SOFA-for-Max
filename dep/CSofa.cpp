//
//  CppSofa.cpp
//  SofaMax2
//
//  Created by Dale Johnson on 28/01/2019.
//  Copyright © 2019 Dale Johnson. All rights reserved.
//

#include "CSofa.hpp"
#include "./SOFA/src/sofa.h"
#include <iostream>
#include <vector>

t_sofaConvention csofa_getConvention(t_sofa* sofa) {
    std::string strConvention = C_SOFA_FILE(sofa->pSofa)->GetSOFAConventions();

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

t_sofa csofa_openFile(char* filename) {
    t_sofa sofa;
    std::string strFileName(filename);
    std::vector<double> dataIR;

    sofa::File file(strFileName);

    sofa.pSofa = &file;
    sofa.convention = csofa_getConvention(&sofa);
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
        default:
            sofa.dataIR = nullptr;
    }

    // Extract positions
    std::vector<double> positions;
    std::vector<double> views;

    C_SOFA_FILE(&file)->GetListenerPosition(positions);
    if(positions.size()) {
        sofa.listenerPoints = new t_point[positions.size()];
        sofa.numListenerPoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.listenerPoints[i].ID = i;
        sofa.listenerPoints[i].pos[0] = positions[i*3];
        sofa.listenerPoints[i].pos[1] = positions[i*3 + 1];
        sofa.listenerPoints[i].pos[2] = positions[i*3 + 2];
    }

    C_SOFA_FILE(&file)->GetReceiverPosition(positions);
    if(positions.size()) {
        sofa.receiverPoints = new t_point[positions.size()];
        sofa.numReceiverPoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.receiverPoints[i].ID = i;
        sofa.receiverPoints[i].pos[0] = positions[i*3];
        sofa.receiverPoints[i].pos[1] = positions[i*3 + 1];
        sofa.receiverPoints[i].pos[2] = positions[i*3 + 2];
    }

    C_SOFA_FILE(&file)->GetSourcePosition(positions);
    if(positions.size()) {
        sofa.sourcePoints = new t_point[positions.size()];
        sofa.numSourcePoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.sourcePoints[i].ID = i;
        sofa.sourcePoints[i].pos[0] = positions[i*3];
        sofa.sourcePoints[i].pos[1] = positions[i*3 + 1];
        sofa.sourcePoints[i].pos[2] = positions[i*3 + 2];
    }

    C_SOFA_FILE(&file)->GetEmitterPosition(positions);
    if(positions.size()) {
        sofa.emitterPoints = new t_point[positions.size()];
        sofa.numEmitterPoints = positions.size() / 3;
    }
    for(auto i = 0; i < positions.size() / 3; ++i) {
        sofa.emitterPoints[i].ID = i;
        sofa.emitterPoints[i].pos[0] = positions[i*3];
        sofa.emitterPoints[i].pos[1] = positions[i*3 + 1];
        sofa.emitterPoints[i].pos[2] = positions[i*3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetListenerView(views);
    if(views.size()) {
        sofa.listenerViews = new t_point[views.size()];
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.listenerViews[i].ID = i;
        sofa.listenerViews[i].pos[0] = views[i*3];
        sofa.listenerViews[i].pos[1] = views[i*3 + 1];
        sofa.listenerViews[i].pos[2] = views[i*3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetSourceView(views);
    if(views.size()) {
        sofa.sourceViews = new t_point[views.size()];
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.sourceViews[i].ID = i;
        sofa.sourceViews[i].pos[0] = views[i*3];
        sofa.sourceViews[i].pos[1] = views[i*3 + 1];
        sofa.sourceViews[i].pos[2] = views[i*3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetReceiverView(views);
    if(views.size()) {
        sofa.receiverViews = new t_point[views.size()];
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.receiverViews[i].ID = i;
        sofa.receiverViews[i].pos[0] = views[i*3];
        sofa.receiverViews[i].pos[1] = views[i*3 + 1];
        sofa.receiverViews[i].pos[2] = views[i*3 + 2];
    }
    
    C_SOFA_FILE(&file)->GetEmitterView(views);
    if(views.size()) {
        sofa.emitterViews = new t_point[views.size()];
    }
    for(auto i = 0; i < views.size() / 3; ++i) {
        sofa.emitterViews[i].ID = i;
        sofa.emitterViews[i].pos[0] = views[i*3];
        sofa.emitterViews[i].pos[1] = views[i*3 + 1];
        sofa.emitterViews[i].pos[2] = views[i*3 + 2];
    }

    sofa.attr = csofa_getAttributes(&sofa);

    return sofa;
}

void csofa_destroySofa(t_sofa* sofaFile) {
    if(sofaFile->dataIR) {
        delete[] (sofaFile->dataIR);
    }
    if(sofaFile->listenerPoints) {
        delete[] sofaFile->listenerPoints;
    }
    csofa_clearAttributes(&sofaFile->attr);
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

uint64_t csofa_getNumAttributes(t_sofa* s) {
    std::vector<std::string> attributeNames;
    C_SOFA_FILE(s->pSofa)->GetAllAttributesNames(attributeNames);
    return attributeNames.size();
}

uint64_t csofa_getMaxAttributeNameSize(t_sofa* s) {
    std::vector<std::string> attributeNames;
    std::vector<std::string>::iterator iter;

    C_SOFA_FILE(s->pSofa)->GetAllAttributesNames(attributeNames);
    iter = std::max_element(attributeNames.begin(), attributeNames.end());
    auto i = std::distance(attributeNames.begin(), iter);
    return attributeNames[i].size();
}

uint64_t csofa_getMaxAttributeSize(t_sofa* s) {
    std::vector<std::string> attributeNames;
    std::string attribute;
    C_SOFA_FILE(s->pSofa)->GetAllAttributesNames(attributeNames);
    uint64_t maxSize = 0;

    for(auto i = 0; i < attributeNames.size(); ++i) {
        attribute = C_SOFA_FILE(s->pSofa)->GetAttributeValueAsString(attributeNames[i]);
        maxSize = attribute.size() > maxSize ? attribute.size() : maxSize;
    }

    return maxSize;
}

t_sofaAttributes csofa_getAttributes(t_sofa* s) {
    t_sofaAttributes a;

    uint64_t numAttributes = csofa_getNumAttributes(s);
    uint64_t maxAttributeNameLength = csofa_getMaxAttributeNameSize(s);
    uint64_t maxAttributeSize = csofa_getMaxAttributeSize(s);

    a.names = (char**)malloc(sizeof(char*) * numAttributes);
    a.values = (char**)malloc(sizeof(char*) * numAttributes);
    a.nameSizes = (uint64_t*)malloc(sizeof(uint64_t) * numAttributes);
    a.valueSizes = (uint64_t*)malloc(sizeof(uint64_t) * numAttributes);
    a.numAttributes = numAttributes;
    a.maxAttributeNameSize= maxAttributeNameLength;
    a.maxAttributeSize = maxAttributeSize;

    for(uint64_t i = 0; i < numAttributes; ++i) {
        a.names[i] = (char*)malloc(sizeof(char*) * maxAttributeNameLength + 1); // +1 allows for null-termination
        a.values[i] = (char*)malloc(sizeof(char*) * maxAttributeSize + 1);
    }

    std::vector<std::string> attributeNames;
    std::string attribute;
    C_SOFA_FILE(s->pSofa)->GetAllAttributesNames(attributeNames);
    for(auto i = 0; i < attributeNames.size(); ++i) {
        attribute = C_SOFA_FILE(s->pSofa)->GetAttributeValueAsString(attributeNames[i]);
        memcpy(a.names[i], attributeNames[i].c_str(), sizeof(char) * attributeNames[i].size());
        memcpy(a.values[i], attribute.c_str(), sizeof(char) * attribute.size());
        a.names[i][attributeNames[i].size()] = '\0';
        a.values[i][attribute.size()] = '\0';
        a.nameSizes[i] = attributeNames[i].size();
        a.valueSizes[i] = attribute.size();
    }

    return a;
}

void csofa_clearAttributes(t_sofaAttributes* a) {
    for(auto i = 0; i < a->numAttributes; ++i) {
        free(a->names[i]);
        free(a->values[i]);
    }
    free(a->names);
    free(a->values);
    free(a->nameSizes);
    free(a->valueSizes);
}
