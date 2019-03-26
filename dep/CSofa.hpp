//
//  CSofa.hpp
//  SofaMax2
//
//  Created by Dale Johnson on 28/01/2019.
//  Copyright © 2019 Dale Johnson. All rights reserved.
//

#ifndef CSofa_hpp
#define CSofa_hpp

#ifdef __cplusplus
extern "C"
{
#endif
#define C_SOFA_FILE reinterpret_cast<sofa::File*>
#define C_SOFA_GENERAL_FIR reinterpret_cast<sofa::GeneralFIR*>
#define C_SOFA_GENERAL_TF reinterpret_cast<sofa::GeneralTF*>
#define C_SOFA_SIMPLE_FF_HRIR reinterpret_cast<sofa::SimpleFreeFieldHRIR*>
#define C_SOFA_GENERAL_FIRE reinterpret_cast<sofa::GeneralFIRE*>
#define C_SOFA_SINGLE_ROOM_DRIR reinterpret_cast<sofa::SingleRoomDRIR*>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "PointTree.h"

    typedef enum _sofaConvention {
        SOFA_GENERAL_FIR,
        SOFA_SIMPLE_FREE_FIELD_HRIR,
        SOFA_GENERAL_FIRE,
        SOFA_SINGLE_ROOM_DRIR,
        SOFA_MULTISPEAKER_BRIR,
        SOFA_UNKNOWN_TYPE
    }t_sofaConvention;

    typedef enum _sofaDimension {
        SOFA_M_DIMENSION = 0,
        SOFA_R_DIMENSION,
        SOFA_E_DIMENSION,
        SOFA_N_DIMENSION,
        SOFA_S_DIMENSION,
        SOFA_NUM_DIMENSIONS
    }t_sofaDimension;

    typedef struct _sofaAttributes {
        char** names;
        char** values;
        uint64_t* nameSizes;
        uint64_t* valueSizes;

        uint64_t numAttributes;
        uint64_t maxAttributeNameSize;
        uint64_t maxAttributeSize;
    }t_sofaAttributes;

    typedef struct _sofa {
        t_sofaConvention convention;
        void* pSofa;
        uint64_t I, M, R, E, N, S, C;

        t_point* listenerPoints;
        t_point* receiverPoints;
        t_point* sourcePoints;
        t_point* emitterPoints;
        
        t_point* listenerViews;
        t_point* receiverViews;
        t_point* sourceViews;
        t_point* emitterViews;

        uint64_t numListenerPoints, numReceiverPoints, numSourcePoints, numEmitterPoints;

        double* dataIR;
        double sampleRate;
        long numBlocks;

        t_sofaAttributes attr;
    }t_sofa;

    t_sofaConvention csofa_getConvention(t_sofa* sofa);

    t_sofa csofa_openFile(char* filename);
    void csofa_destroySofa(t_sofa* sofa);

////////////////////////////////////////////////////////////////////////////////////////////////////

    double* csofa_getDataIR(t_sofa* s, uint64_t i);
    double* csofa_getMRDataIR(t_sofa* s, uint64_t M, uint64_t R);
    double* csofa_getSimpleFreeFieldHRIRDataIR(t_sofa* s, uint64_t M, uint64_t R);
    double* csofa_getGeneralFIRDataIR(t_sofa* s, uint64_t M, uint64_t R);

    double* csofa_getGeneralFIREDataIR(t_sofa* s, uint64_t M, uint64_t R, uint64_t E);
    double* csofa_getMultiSpeakerBRIR(t_sofa* s, uint64_t M, uint64_t R, uint64_t E);

////////////////////////////////////////////////////////////////////////////////////////////////////

    uint64_t csofa_getNumAttributes(t_sofa* s);
    uint64_t csofa_getMaxAttributeNameSize(t_sofa* s);
    uint64_t csofa_getMaxAttributeSize(t_sofa* s);

    void csofa_getAttributes(t_sofa* s, t_sofaAttributes* a);
    void csofa_clearAttributes(t_sofaAttributes* a);
    
////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void csofa_setDataIR(t_sofa* s, uint64_t i, double* data);


#ifdef __cplusplus
}
#endif

#endif /* CppSofa_hpp */
