//
//  sofa_common.c
//  SofaMax
//
//  Created by Dale Johnson on 28/02/2019.
//
//

#include "sofa_common.h"

char* sofa_getConventionString(t_sofaConvention convention) {
    char* strConvention;
    switch(convention) {
        case SOFA_GENERAL_FIR: strConvention = "GeneralFIR"; break;
        case SOFA_SIMPLE_FREE_FIELD_HRIR: strConvention = "SimpleFreeFieldHRIR"; break;
        case SOFA_GENERAL_FIRE: strConvention = "GeneralFIRE"; break;
        case SOFA_SINGLE_ROOM_DRIR: strConvention = "SingleRoomDRIR"; break;
        case SOFA_MULTISPEAKER_BRIR: strConvention = "MultiSpeakerBRIR"; break;
        default: strConvention = "unknown";
    }
    return strConvention;
}

bool isSofaFileOpen(t_object* ob, t_sofa_max* x, t_symbol* s) {
    if(!*x->fileLoaded) {
        object_error(ob, "%s: no SOFA file open", s->s_name);
        return false;
    }
    return true;
}

long sofa_hashAttributeType(t_symbol* s) {
    if(s == gensym("listener")) {
        return 1;
    }
    if(s == gensym("receiver")) {
        return 2;
    }
    if(s == gensym("source")) {
        return 3;
    }
    if(s == gensym("emitter")) {
        return 4;
    }
    return 0;
}

void sofa_getPositions(t_sofa_max* x, void* outlet, t_symbol* s) {
    t_symbol* mess = gensym("getpositions");
    if(sofa_hashAttributeType(s)) {
        long dim = 3;
        if(x->sofa->convention == SOFA_SIMPLE_FREE_FIELD_HRIR) {
            dim = 2;
        }
        sofa_dumpPositions(x, outlet, mess, s, dim);
    }
    else {
        object_error((t_object*)x, "%s: requested invalid position type", mess);
        return;
    }
}

void sofa_dumpPositions(t_sofa_max* x, void* outlet, t_symbol* s, t_symbol* p, long dim) {
    t_atom argv[4];
    uint64_t numPoints = 0;
    t_point* points;
    t_symbol* mess;
    switch(sofa_hashAttributeType(p)) {
        case LISTENER_POSITION:
            numPoints = x->sofa->numListenerPoints;
            points = x->sofa->listenerPoints;
            mess = gensym("listenerpos");
            break;
        case RECEIVER_POSITION:
            numPoints = x->sofa->numReceiverPoints;
            points = x->sofa->receiverPoints;
            mess = gensym("receiverpos");
            break;
        case SOURCE_POSITION:
            numPoints = x->sofa->numSourcePoints;
            points = x->sofa->sourcePoints;
            mess = gensym("sourcepos");
            break;
        case EMITTER_POSITION:
            numPoints = x->sofa->numEmitterPoints;
            points = x->sofa->emitterPoints;
            mess = gensym("emitterpos");
            break;
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
    if(sofa_hashAttributeType(s)) {
        sofa_dumpViews(x, outlet, mess, s);
    }
    else {
        object_error((t_object*)x, "%s: requested invalid view type", mess);
        return;
    }
}

void sofa_dumpViews(t_sofa_max* x, void* outlet, t_symbol* s, t_symbol* p) {
    t_atom argv[4];
    uint64_t numPoints = 0;
    t_point* points;
    t_symbol* mess;
    switch(sofa_hashAttributeType(p)) {
        case LISTENER_POSITION:
            numPoints = x->sofa->numListenerPoints;
            points = x->sofa->listenerPoints;
            mess = gensym("listenerview");
            break;
        case RECEIVER_POSITION:
            numPoints = x->sofa->numReceiverPoints;
            points = x->sofa->receiverPoints;
            mess = gensym("receiverview");
            break;
        case SOURCE_POSITION:
            numPoints = x->sofa->numSourcePoints;
            points = x->sofa->sourcePoints;
            mess = gensym("sourceview");
            break;
        case EMITTER_POSITION:
            numPoints = x->sofa->numEmitterPoints;
            points = x->sofa->emitterPoints;
            mess = gensym("emitterview");
            break;
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
