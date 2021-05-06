//
//  sofa_common.c
//  SofaMax
//
//  Created by Dale Johnson on 28/02/2019.
//
//

#include "sofa_common.h"

const char* sofa_getConventionString(t_sofaConvention convention) {
//    switch(convention) {
//        case SOFA_GENERAL_FIR: strConvention = "GeneralFIR"; break;
//        case SOFA_SIMPLE_FREE_FIELD_HRIR: strConvention = "SimpleFreeFieldHRIR"; break;
//        case SOFA_GENERAL_FIRE: strConvention = "GeneralFIRE"; break;
//        case SOFA_SINGLE_ROOM_DRIR: strConvention = "SingleRoomDRIR"; break;
//        case SOFA_MULTISPEAKER_BRIR: strConvention = "MultiSpeakerBRIR"; break;
//        default: strConvention = "unknown";
//    }
    if (convention < SOFA_NUM_CONVENTIONS) {
        return kStrConventions[(long)convention];
    }
    return "unknown";
}

t_sofaConvention sofa_getConventionFromString(char* str) {
    t_sofaConvention convention = SOFA_UNKNOWN_TYPE;
    long i;
    for(i = 0; i < SOFA_NUM_CONVENTIONS; ++i) {
        if (strcmp(str, kStrConventions[i]) == 0) {
            convention = (t_sofaConvention)i;
        }
    }
    return convention;
}

bool isSofaFileOpen(t_object* ob, t_sofa_max* x, t_symbol* s) {
    if(!*x->fileLoaded) {
        object_error(ob, "%s: no SOFA file open", s->s_name);
        return false;
    }
    return true;
}

t_sofaVarType sofa_hashVarTypeFromName(t_symbol* s) {
    if(s == gensym("listener")) {
        return SOFA_VAR_LISTENER;
    }
    if(s == gensym("receiver")) {
        return SOFA_VAR_RECEIVER;
    }
    if(s == gensym("source")) {
        return SOFA_VAR_SOURCE;
    }
    if(s == gensym("emitter")) {
        return SOFA_VAR_EMITTER;
    }
    return SOFA_VAR_UNKNOWN;
}

void sofa_getPositions(t_sofa_max* x, void* outlet, t_symbol* s) {
    t_symbol* mess = gensym("getpositions");
    if(sofa_hashVarTypeFromName(s) == SOFA_VAR_UNKNOWN) {
        object_error((t_object*)x, "%s: requested invalid position type", mess);
        return;
    }
    else {
        long dim = 3;
        if(x->sofa->convention == SOFA_SIMPLE_FREE_FIELD_HRIR) {
            dim = 2;
        }
        sofa_dumpPositions(x, outlet, mess, s, dim);
    }
}

void sofa_dumpPositions(t_sofa_max* x, void* outlet, t_symbol* s, t_symbol* p, long dim) {
    t_atom argv[4];
    uint64_t numPoints = 0;
    t_point* points;
    t_symbol* mess;
    switch(sofa_hashVarTypeFromName(p)) {
        case SOFA_VAR_LISTENER:
            numPoints = x->sofa->numListenerPoints;
            points = x->sofa->listenerPoints;
            mess = gensym("listenerpos");
            break;
        case SOFA_VAR_RECEIVER:
            numPoints = x->sofa->numReceiverPoints;
            points = x->sofa->receiverPoints;
            mess = gensym("receiverpos");
            break;
        case SOFA_VAR_SOURCE:
            numPoints = x->sofa->numSourcePoints;
            points = x->sofa->sourcePoints;
            mess = gensym("sourcepos");
            break;
        case SOFA_VAR_EMITTER:
            numPoints = x->sofa->numEmitterPoints;
            points = x->sofa->emitterPoints;
            mess = gensym("emitterpos");
            break;
        default:
            return;
    }

    for(long i = 0; i < numPoints; ++i) {
        atom_setlong(&argv[0], points[i].ID);
        for(long d = 0; d < dim; ++d) {
            atom_setfloat(&argv[d + 1], points[i].pos[d]);
        }
        outlet_anything(outlet, mess, dim + 1, argv);
    }
}

void sofa_getViews(t_sofa_max* x, void* outlet, t_symbol* s) {
    t_symbol* mess = gensym("getviews");
    if(sofa_hashVarTypeFromName(s) == SOFA_VAR_UNKNOWN) {
        object_error((t_object*)x, "%s: requested invalid view type", mess);
        return;
    }
    else {
        sofa_dumpViews(x, outlet, mess, s);
    }
}

void sofa_dumpViews(t_sofa_max* x, void* outlet, t_symbol* s, t_symbol* p) {
    t_atom argv[4];
    uint64_t numPoints = 0;
    t_point* points;
    t_symbol* mess;
    switch(sofa_hashVarTypeFromName(p)) {
        case SOFA_VAR_LISTENER:
            numPoints = x->sofa->numListenerPoints;
            points = x->sofa->listenerPoints;
            mess = gensym("listenerview");
            break;
        case SOFA_VAR_RECEIVER:
            numPoints = x->sofa->numReceiverPoints;
            points = x->sofa->receiverPoints;
            mess = gensym("receiverview");
            break;
        case SOFA_VAR_SOURCE:
            numPoints = x->sofa->numSourcePoints;
            points = x->sofa->sourcePoints;
            mess = gensym("sourceview");
            break;
        case SOFA_VAR_EMITTER:
            numPoints = x->sofa->numEmitterPoints;
            points = x->sofa->emitterPoints;
            mess = gensym("emitterview");
            break;
        default:
            return;
    }

    for(long i = 0; i < numPoints; ++i) {
        atom_setlong(&argv[0], points[i].ID);
        for(long d = 0; d < 3; ++d) {
            atom_setfloat(&argv[d + 1], points[i].pos[d]);
        }
        outlet_anything(outlet, mess, 4, argv);
    }
}

t_symbol* sofa_getAttributeValueByName(t_sofa_max* x, t_symbol* name, long* attrid) {
    t_symbol* val = NULL;
    long i = 0;
    if (x && name) {
        do {
            if (strcmp(name->s_name, kStrAttr[i]) == 0) {
                val = x->attributes[i];
                *attrid = i;
            }
            ++i;
        } while(val == NULL && i < NUM_ATTR_TYPES);
    }
    return val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool atomIsANumber(t_atom* a) {
    if(atom_gettype(a) == A_LONG || atom_gettype(a) == A_FLOAT) {
        return true;
    }
    return false;
}

bool atomIsASymbol(t_atom* a) {
    if(atom_gettype(a) == A_SYM) {
        return true;
    }
    return false;
}
