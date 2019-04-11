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
    
    // Attributes
    t_symbol* version;
    t_symbol* convention;
    t_symbol* conventionVersion;
    t_symbol* dataType;
    t_symbol* roomType;
    t_symbol* title;
    t_symbol* dateCreated;
    t_symbol* dateModified;
    t_symbol* apiName;
    t_symbol* apiVersion;
    t_symbol* authorContact;
    t_symbol* organization;
    t_symbol* license;
    t_symbol* applicationName;
    t_symbol* applicationVersion;
    t_symbol* comment;
    t_symbol* history;
    t_symbol* references;
    t_symbol* origin;
    
    bool* fileLoaded;
    long* count;
    
    t_buffer_ref* buffRef;
    
    void* outlet_finishedLoading;
    void* outlet_dump;
}t_sofa_max;

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
