//
//  sofa_common.h
//  SofaMax
//
//  Created by Dale Johnson on 04/02/2019.
//
//

#ifndef sofa_common_h
#define sofa_common_h
#define APL_SOFA_NAMESPACE gensym("apl.sofa")
#define APL_SOFA_CLASSNAME gensym("sofa~")

#include "../dep/CSofa.hpp"
#include "ext.h"
#include "ext_buffer.h"
#include "ext_critical.h"

typedef struct _sofa_max {
    t_object ob;
    t_symbol* name;
    t_sofa* sofa;
    
    bool* fileLoaded;
    long* count;
    
    t_buffer_ref* buffRef;
    
    void* outlet_finishedLoading;
    void* outlet_dump;
}t_sofa_max;

typedef enum _positionType {
    UNKNOWN_POSITION = 0,
    LISTENER_POSITION,
    RECEIVER_POSITION,
    SOURCE_POSITION,
    EMITTER_POSITION
}t_positionType;

char* sofa_getConventionString(t_sofaConvention convention);

bool isSofaFileOpen(t_object* ob, t_sofa_max* x, t_symbol* s);

long sofa_hashAttributeType(t_symbol* s);

void sofa_getPositions(t_sofa_max* x, void* outlet, t_symbol* s);
void sofa_dumpPositions(t_sofa_max* x, void* outlet, t_symbol* s, t_symbol* p, long dim);
void sofa_getViews(t_sofa_max* x, void* outlet, t_symbol* s);
void sofa_dumpViews(t_sofa_max* x, void* outlet, t_symbol* s, t_symbol* p);

////////////////////////////////////////////////////////////////////////////////////////////////////

bool atomIsANumber(t_atom* a);
bool atomIsASymbol(t_atom* a);

#endif /* sofa_common_h */
