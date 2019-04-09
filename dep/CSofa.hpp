//
//  CSofa.hpp
//  SofaMax2
//
//  Created by Dale Johnson on 28/01/2019.
//  Copyright © 2019 Dale Johnson. All rights reserved.
//

#ifndef CSofa_hpp
#define CSofa_hpp

#define C_SOFA_FILE reinterpret_cast<sofa::File*>
#define C_SOFA_GENERAL_FIR reinterpret_cast<sofa::GeneralFIR*>
#define C_SOFA_GENERAL_TF reinterpret_cast<sofa::GeneralTF*>
#define C_SOFA_SIMPLE_FF_HRIR reinterpret_cast<sofa::SimpleFreeFieldHRIR*>
#define C_SOFA_GENERAL_FIRE reinterpret_cast<sofa::GeneralFIRE*>
#define C_SOFA_SINGLE_ROOM_DRIR reinterpret_cast<sofa::SingleRoomDRIR*>
#define C_SOFA_MULTISPEAKER_BRIR reinterpret_cast<sofa::MultiSpeakerBRIR*>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "PointTree.h"

#ifdef __cplusplus
extern "C" {
#endif
    typedef enum _sofaConvention {
        SOFA_GENERAL_FIR = 0,
        SOFA_SIMPLE_FREE_FIELD_HRIR,
        SOFA_GENERAL_FIRE,
        SOFA_SINGLE_ROOM_DRIR,
        SOFA_MULTISPEAKER_BRIR,
        
        SOFA_NUM_CONVENTIONS,
        
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
    
    typedef enum _positionType {
        UNKNOWN_POSITION = 0,
        LISTENER_POSITION,
        RECEIVER_POSITION,
        SOURCE_POSITION,
        EMITTER_POSITION
    }t_positionType;
    
    typedef enum _viewType {
        UNKNOWN_VIEW = 0,
        LISTENER_VIEW,
        SOURCE_VIEW,
        EMITTER_VIEW
    }t_viewType;
    
    typedef enum _upType {
        UNKNOWN_UP = 0,
        LISTENER_UP,
        RECEIVER_UP,
        SOURCE_UP,
        EMITTER_UP
    }t_upType;
    
    typedef enum _sofaAttributeTypes {
        CONVENTIONS_ATTR_TYPE = 0,
        VERSION_ATTR_TYPE,
        SOFA_CONVENTION_ATTR_TYPE,
        SOFA_CONVENTION_VERSION_ATTR_TYPE,
        DATA_ATTR_TYPE,
        ROOM_ATTR_TYPE,
        TITLE_ATTR_TYPE,
        DATE_CREATED_ATTR_TYPE,
        DATE_MODIFIED_ATTR_TYPE,
        API_NAME_ATTR_TYPE,
        API_VERSION_ATTR_TYPE,
        AUTHOR_CONTACT_ATTR_TYPE,
        ORGANIZATION_ATTR_TYPE,
        LICENSE_ATTR_TYPE,
        APPLICATION_NAME_ATTR_TYPE,
        APPLICATION_VERSION_ATTR_TYPE,
        COMMENT_ATTR_TYPE,
        HISTORY_ATTR_TYPE,
        REFERENCES_ATTR_TYPE,
        ORIGIN_ATTR_TYPE,
        
        ROOM_SHORTNAME_ATTR_TYPE,
        ROOM_DESCRIPTION_ATTR_TYPE,
        ROOM_LOCATION_ATTR_TYPE,
        
        LISTENER_SHORTNAME_ATTR_TYPE,
        LISTENER_DESCRIPTION_ATTR_TYPE,
        
        SOURCE_SHORTNAME_ATTR_TYPE,
        SOURCE_DESCRIPTION_ATTR_TYPE,
        
        RECEIVER_SHORTNAME_ATTR_TYPE,
        RECEIVER_DESCRIPTION_ATTR_TYPE,
        
        EMITTER_SHORTNAME_ATTR_TYPE,
        EMITTER_DESCRIPTION_ATTR_TYPE,
        
        NUM_ATTR_TYPES,
    }t_sofaAttributeTypes;

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
        uint64_t I, M, R, E, N, S, C;

        t_point* listenerPoints;
        t_point* receiverPoints;
        t_point* sourcePoints;
        t_point* emitterPoints;
        
        t_point* listenerViews;
        t_point* receiverViews;
        t_point* sourceViews;
        t_point* emitterViews;
        
        t_point* listenerUps;
        t_point* receiverUps;
        t_point* sourceUps;
        t_point* emitterUps;

        uint64_t numListenerPoints, numReceiverPoints, numSourcePoints, numEmitterPoints;
        uint64_t numListenerViews, numReceiverViews, numSourceViews, numEmitterViews;
        uint64_t numListenerUps, numReceiverUps, numSourceUps, numEmitterUps;

        double* dataIR;
        double sampleRate;
        long numBlocks;

        t_sofaAttributes attr;
    }t_sofa;

    t_sofa csofa_openFile(char* filename);
    t_sofa csofa_newSofa(t_sofaConvention convention, long M, long R, long E, long N, double sampleRate);
    void csofa_destroySofa(t_sofa* sofa);
    
    typedef enum _sofaWriteErr {
        NO_WRITE_ERROR = 0,
        GENERAL_WRITE_ERROR,
        MISSING_ATTR_ERROR,
    }t_sofaWriteErr;
    
    bool csofa_sofaFileExists(const char* filename);
    t_sofaWriteErr csofa_writeFile(const t_sofa* sofa, const char* filename);
    bool csofa_hasRequiredAttributes(const t_sofa* s);

////////////////////////////////////////////////////////////////////////////////////////////////////

    double* csofa_getDataIR(t_sofa* s, uint64_t i);
    double* csofa_getMRDataIR(t_sofa* s, uint64_t M, uint64_t R);
    double* csofa_getSimpleFreeFieldHRIRDataIR(t_sofa* s, uint64_t M, uint64_t R);
    double* csofa_getGeneralFIRDataIR(t_sofa* s, uint64_t M, uint64_t R);

    double* csofa_getGeneralFIREDataIR(t_sofa* s, uint64_t M, uint64_t R, uint64_t E);
    double* csofa_getMultiSpeakerBRIR(t_sofa* s, uint64_t M, uint64_t R, uint64_t E);

////////////////////////////////////////////////////////////////////////////////////////////////////
    
    void csofa_newAttributes(t_sofaAttributes* a);
    void csofa_clearAttributes(t_sofaAttributes* a);
    void csofa_setAttributeValue(t_sofaAttributes* a, t_sofaAttributeTypes t, char* value, int size);
    
////////////////////////////////////////////////////////////////////////////////////////////////////
    
    typedef enum _sofaSetDataErr {
        SET_DATA_NO_ERROR = 0,
        
        SET_DATA_INPUT_NULL_ERROR,
        SET_DATA_SOFA_NULL_ERROR,
        SET_DATA_SOFA_DATA_NULL_ERROR,
        
        SET_DATA_INPUT_TOO_BIG_ERROR,
        SET_DATA_BLOCK_OUT_OF_RANGE_ERROR
    }t_sofaSetDataErr;
    
    t_sofaSetDataErr csofa_setRawDataBlock(t_sofa* s, uint64_t i, double* dataBlock, long N);
    t_sofaSetDataErr csofa_setMRDataBlock(t_sofa* s, uint64_t M, uint64_t R, double* dataBlock,
                                          long N);
    t_sofaSetDataErr csofa_setMREDataBlock(t_sofa* s, uint64_t M, uint64_t R, uint64_t E,
                                           double* dataBlock, long N);
    
    typedef enum _sofaVarType {
        SOFA_VAR_LISTENER = 0,
        SOFA_VAR_RECEIVER,
        SOFA_VAR_SOURCE,
        SOFA_VAR_EMITTER,
        SOFA_NUM_VAR_TYPES
    }t_sofaVarType;
    
    bool csofa_setPosition(t_sofa* s, t_sofaVarType varType, uint64_t i, t_point* point);
    bool csofa_setView(t_sofa* s, t_sofaVarType varType, uint64_t i, t_point* view);
    bool csofa_setUp(t_sofa* s, t_sofaVarType varType, uint64_t i, t_point* up);
    
#ifdef __cplusplus
}
#endif

#endif /* CppSofa_hpp */
