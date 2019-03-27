/**
	@file
	sofa~ - A SOFA importer for Max
	Dale Johnson - Dale.Johnson@hud.ac.uk

	@ingroup	examples
*/

#include "../dep/sofa_common.h"
#include "ext.h"

#include "ext_common.h"
#include "ext_globalsymbol.h"
#include "z_dsp.h"
#include <stdlib.h>

typedef struct _sofa_hrir {
    t_object ob;
    t_sofa_max* sofa_ob;
    t_symbol* sofa_name;
    bool isBoundToSofa;

    struct kdtree* tree;
    t_point* sourcePositions;
    t_point* nearestPosition;
    double* dataIR;

    t_buffer_ref* buffRef;
    t_symbol* buffName;
    bool buffSet;

    void* outlet_dump;
}t_sofa_hrir;

void *sofa_hrir_new(t_symbol *s, long argc, t_atom *argv);
void sofa_hrir_free(t_sofa_hrir *x);

void sofa_hrir_setSofa(t_sofa_hrir* x, t_symbol* s);
void sofa_hrir_setBuffer(t_sofa_hrir* x, t_symbol *s);
bool sofa_hrir_registerBuffer(t_sofa_hrir* x, t_symbol *s);
void sofa_hrir_get(t_sofa_hrir* x, t_symbol* s, long argc, t_atom *argv);
double* sofa_hrir_getHRIR(t_sofa_hrir* x, t_point* p);

void sofa_hrir_createSourceTree(t_sofa_hrir* x);
void sofa_hrir_getPositions(t_sofa_hrir* x, t_symbol* s);
void sofa_hrir_getSize(t_sofa_hrir* x);
void sofa_hrir_getSizeSamps(t_sofa_hrir* x);

bool sofa_hrir_isSofaValid(t_sofa_hrir* x, t_symbol* s);
t_max_err sofa_hrir_notify(t_sofa_hrir *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
t_max_err sofa_hrir_attrSetSofa(t_sofa_hrir *x, t_object *attr, long argc, t_atom *argv);
t_max_err sofa_hrir_attrGetSofa(t_sofa_hrir *x, t_object *attr, long *argc, t_atom **argv);

void sofa_hrir_assist(t_sofa_hrir *x, void *b, long m, long a, char *s);

void *sofa_hrir_class;


void ext_main(void *r) {
	t_class *c;

	c = class_new("sofa.hrir~", (method)sofa_hrir_new, (method)sofa_hrir_free, (long)sizeof(t_sofa_hrir), 0L, A_GIMME, 0);
    
    class_addmethod(c, (method)sofa_hrir_getSize,       "getsize",             A_NOTHING, 0);
    class_addmethod(c, (method)sofa_hrir_getSize,       "getsize",             A_NOTHING, 0);
    class_addmethod(c, (method)sofa_hrir_getSizeSamps,  "getsizesamps",        A_NOTHING, 0);

    class_addmethod(c, (method)sofa_hrir_setSofa,       "set",                 A_SYM, 0);
    class_addmethod(c, (method)sofa_hrir_setBuffer,     "setbuffer",           A_SYM, 0);
    class_addmethod(c, (method)sofa_hrir_get,           "get",                 A_GIMME, 0);
    class_addmethod(c, (method)sofa_hrir_getPositions,  "getpositions",        A_SYM, 0);

    class_addmethod(c, (method)sofa_hrir_notify,        "notify",              A_CANT, 0);
	class_addmethod(c, (method)sofa_hrir_assist,        "assist",              A_CANT, 0);

    CLASS_ATTR_SYM(c, "sofaobject", 0, t_sofa_hrir, sofa_name);
    CLASS_ATTR_ACCESSORS(c, "sofaobject", sofa_hrir_attrGetSofa, sofa_hrir_attrSetSofa);
    CLASS_ATTR_SELFSAVE(c, "sofaobject", 0);
    CLASS_ATTR_LABEL(c, "sofaobject", 0, "sofa~ object");

	class_register(CLASS_BOX, c);
	sofa_hrir_class = c;
}

void sofa_hrir_setSofa(t_sofa_hrir* x, t_symbol* s) {
    t_sofa_max* ref = (t_sofa_max*)globalsymbol_reference((t_object*)x, s->s_name, "sofa~");
    if(ref != NULL) {
        x->sofa_ob = ref;
        object_subscribe(APL_SOFA_NAMESPACE, s, APL_SOFA_CLASSNAME, x);
        x->isBoundToSofa = true;
        sofa_hrir_createSourceTree(x);
    }
}

void sofa_hrir_setBuffer(t_sofa_hrir* x, t_symbol* s) {
    if(!sofa_hrir_registerBuffer(x, s)) {
        object_error((t_object*)x, "setbuffer: buffer \"%s\" does not exist", s->s_name);
    }
}

bool sofa_hrir_registerBuffer(t_sofa_hrir* x, t_symbol* s) {
    t_buffer_ref* buffRefTemp = buffer_ref_new((t_object*)x, s);
    if(buffer_ref_exists(buffRefTemp) == 0) {
        object_free(buffRefTemp);
        return false;
    }

    if(x->buffRef) {
        buffer_ref_set(x->buffRef, s);
    }
    else {
        x->buffRef = buffRefTemp;
    }
    x->buffName = s;
    return true;
}

void sofa_hrir_getSize(t_sofa_hrir* x) {
    if(x->sofa_ob) {
        if(x->sofa_ob->sofa->N) {
            t_atom argv;
            atom_setfloat(&argv, x->sofa_ob->sofa->N / sys_getsr() * 1000.f);
            outlet_anything(x->outlet_dump, gensym("size"), 1, &argv);
        }
    }
}

void sofa_hrir_getSizeSamps(t_sofa_hrir* x) {
    if(x->sofa_ob) {
        if(x->sofa_ob->sofa->N) {
            t_atom argv;
            atom_setlong(&argv, x->sofa_ob->sofa->N);
            outlet_anything(x->outlet_dump, gensym("sizesamps"), 1, &argv);
        }
    }
}

void sofa_hrir_assist(t_sofa_hrir *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
        switch(a) {
            case 0:
                sprintf(s, "(anything) Message inlet");
                break;
        }
	}
	else {
        switch(a) {
            case 0:
                sprintf(s, "(aynthing) Message outlet");
                break;
        }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HRTF Extraction
////////////////////////////////////////////////////////////////////////////////////////////////////

void sofa_hrir_get(t_sofa_hrir* x, t_symbol* s, long argc, t_atom *argv) {

    // Check arguments
    if(argc < 2) {
        object_error((t_object*)x, "%s: not enough arguments", s->s_name);
        return;
    }

    for(uint32_t i = 0 ; i < 2; ++i) {
        if(atom_gettype(argv + i) != A_FLOAT && atom_gettype(argv + i) != A_LONG) {
            object_error((t_object*)x, "%s: position argument %d should be number", s->s_name, i + 1);
            return;
        }
    }

    t_point p;
    p.pos[0] = atom_getfloat(argv);
    argv++;
    p.pos[1] = atom_getfloat(argv);
    argv++;
    p.pos[2] = 0;

    if(!sofa_hrir_isSofaValid(x, s)) {
        return;
    }

    t_symbol* optionalBufferName = NULL;
    bool optionalBufferIsGiven = false;
    if(argc > 2) {
        if(atom_gettype(argv) != A_SYM) {
            object_error((t_object*)x, "%s: destination buffer name should be a symbol");
            return;
        }
        optionalBufferIsGiven = true;
        optionalBufferName = atom_getsym(argv);
        argv++;
    }

    long receiverNumber = 0;
    bool optionalReceiverNumberGiven = false;
    if(argc > 3) {
        if(!atomIsANumber(argv)) {
            object_error((t_object*)x, "%s: receiver should be a number", s->s_name);
            return;
        }
        optionalReceiverNumberGiven = true;
        receiverNumber = atom_getlong(argv);
        if(receiverNumber < 0) {
            receiverNumber = 0;
        }
        if(receiverNumber > x->sofa_ob->sofa->R) {
            receiverNumber = x->sofa_ob->sofa->R;
        }
        argv++;
    }

    long buffChanOffset = 0;
    bool optionalBuffChanOffsetGiven = false;
    if(argc > 4) {
        if(!atomIsANumber(argv)) {
            object_error((t_object*)x, "%s: buffer channel offset should be a number", s->s_name);
            return;
        }
        optionalBuffChanOffsetGiven = true;
        buffChanOffset = atom_getlong(argv);
        argv++;
    }

    if(argc > 5) {
        object_warn((t_object*)x, "extra arguments for message \"%s\"", s->s_name);
    }

    // Buffer operations

    t_buffer_ref* buffRef;
    t_buffer_obj* buffObj;

    if(optionalBufferIsGiven) {
        buffRef = buffer_ref_new((t_object*)x, optionalBufferName);
        if(buffer_ref_exists(buffRef) == 0) {
            object_error((t_object*)x, "%s: destination buffer \"%s\" does not exist", s->s_name, optionalBufferName->s_name);
            object_free(buffRef);
            return;
        }
    }
    /*else if(buffRef == NULL) {
        object_error((t_object*)x, "%s: no destination buffer has been set", s->s_name);
        return;
    }*/
    else {
        if(!x->buffSet) {
            x->buffSet = sofa_hrir_registerBuffer(x, x->buffName);
            if(!x->buffSet) {
                object_error((t_object*)x, "%s: no destination buffer has been set", s->s_name);
                return;
            }
        }
        buffRef = x->buffRef;
    }

    buffObj = buffer_ref_getobject(buffRef);

    long numBuffChans = buffer_getchannelcount(buffObj);
    long numBuffFrames = buffer_getframecount(buffObj);

    if(optionalBuffChanOffsetGiven) {
        if(buffChanOffset > (numBuffChans - 1) || buffChanOffset < 0 ) {
            if(optionalBufferIsGiven && buffRef) {
                object_free(buffRef);
            }
            return;
        }
    }

    if(numBuffFrames < x->sofa_ob->sofa->N) {
        object_error((t_object*)x, "%s: destination buffer is too short", s->s_name);
        if(optionalBufferIsGiven && buffRef) {
            object_free(buffRef);
        }
        return;
    }

    long numValidChannels = numBuffChans;
    long receiverChanOffset = 0;
    if(numValidChannels > x->sofa_ob->sofa->R) {
        numValidChannels = x->sofa_ob->sofa->R;
    }

    if(optionalReceiverNumberGiven) {
        numValidChannels = 1;
        if(receiverNumber == 2) {
            receiverChanOffset = 1;
        }

    }
    if(optionalBuffChanOffsetGiven && buffChanOffset > (numBuffChans - 2)) {
        numValidChannels = 1;
    }

    // Do extraction

    double* d = sofa_hrir_getHRIR(x, &p);
    if(d == NULL) {
        object_error((t_object*)x, "%s: data extraction was unsuccesful", s->s_name);
        if(optionalBufferIsGiven && buffRef) {
            object_free(buffRef);
        }
        return;
    }

    float* buffData = buffer_locksamples(buffObj);
    long buffIndex = 0;
    long dataIndex = 0;
    long N = x->sofa_ob->sofa->N;
    if(buffData) {
        for(long i = 0; i < N; ++i) {
            for(long j = 0; j < numValidChannels; ++j) {
                buffIndex = (i * numBuffChans) + j + buffChanOffset;
                dataIndex = i + (j + receiverChanOffset) * N;
                buffData[buffIndex] = d[dataIndex];
            }
        }
        buffer_unlocksamples(buffObj);
        buffer_setdirty(buffObj);
    }
    else {
        buffer_unlocksamples(buffObj);
    }

    if(optionalBufferIsGiven && buffRef) {
        object_free(buffRef);
    }
    
    t_atom pos[2];
    atom_setfloat(&pos[0], x->nearestPosition->pos[0]);
    atom_setfloat(&pos[1], x->nearestPosition->pos[1]);
    outlet_anything(x->outlet_dump, gensym("position"), 2, pos);
}

double* sofa_hrir_getHRIR(t_sofa_hrir* x, t_point* p) {
    if(x->tree) {
        x->nearestPosition = getNearestPoint(x->tree, p);
        if(x->nearestPosition) {
            return csofa_getSimpleFreeFieldHRIRDataIR(x->sofa_ob->sofa, x->nearestPosition->ID, 0);
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Tree generation
////////////////////////////////////////////////////////////////////////////////////////////////////

void sofa_hrir_createSourceTree(t_sofa_hrir* x) {
    if(x->sofa_ob) {
        if(*x->sofa_ob->fileLoaded == false) {
            return;
        }

        if(x->sofa_ob->sofa->convention != SOFA_SIMPLE_FREE_FIELD_HRIR) {
            return;
        }

        if(x->sourcePositions) {
            sysmem_freeptr(x->sourcePositions);
        }
        if(x->tree) {
            kd_free(x->tree);
        }

        // Get source positions
        long numSourcePositions = x->sofa_ob->sofa->M;
        if(numSourcePositions) {
            x->tree = kd_create(2);
            for(uint32_t i = 0; i < numSourcePositions; ++i) {
                kd_insert(x->tree, x->sofa_ob->sofa->sourcePoints[i].pos, &x->sofa_ob->sofa->sourcePoints[i]);
            }
        }
    }
}

void sofa_hrir_getPositions(t_sofa_hrir* x, t_symbol* s) {
    if(!sofa_hrir_isSofaValid(x, gensym("getpositions"))) {
        return;
    }
    sofa_getPositions(x->sofa_ob, x->outlet_dump, s);
}

bool sofa_hrir_isSofaValid(t_sofa_hrir* x, t_symbol* mess) {
    if(!x->isBoundToSofa) {
        object_error((t_object*)x, "%s: no sofa~ object set", mess->s_name);
        return false;
    }
    if(!isSofaFileOpen((t_object*)x, x->sofa_ob, mess)) {
        return false;
    }
    if(x->sofa_ob->sofa->convention != SOFA_SIMPLE_FREE_FIELD_HRIR) {
        object_error((t_object*)x, "%s: incompatible convention. sofa.hrir~ expects a SimpleFreeFieldHRIR", mess->s_name);
        return false;
    }
    return true;
}

t_max_err sofa_hrir_notify(t_sofa_hrir *x, t_symbol *s, t_symbol *msg, void *sender, void *data) {
    t_max_err err = MAX_ERR_NONE;
    t_symbol* buffName = (t_symbol*)object_method(sender, gensym("getname"));
    
    if(msg == gensym("globalsymbol_binding")) {
        if(x->sofa_name) {
            if(object_findregistered(APL_SOFA_NAMESPACE, x->sofa_name)) {
                sofa_hrir_setSofa(x, x->sofa_name);
            }
        }
    }

    if(msg == gensym("willfree") && object_classname_compare(sender, APL_SOFA_CLASSNAME)) {
        if(((t_sofa_max*)sender)->name == x->sofa_name) {
            object_unsubscribe(APL_SOFA_NAMESPACE, x->sofa_name, APL_SOFA_CLASSNAME, x);
            x->sofa_ob = NULL;
            x->isBoundToSofa = false;
        }
    }

    if(msg == gensym("sofaread")) {
        if(x->isBoundToSofa) {
            if(((t_sofa_max*)sender)->name == x->sofa_name) {
                sofa_hrir_createSourceTree(x);
            }
        }
    }
    
    if(buffName) {
        if(buffName == x->buffName) {
            if(x->buffRef) {
                err = buffer_ref_notify(x->buffRef, s, msg, sender, data);
                if(buffer_ref_exists(x->buffRef) == 0) {
                    object_free(x->buffRef);
                    x->buffRef = NULL;
                    x->buffSet = false;
                }
            }
        }
    }
    
    /*if(buffName) {
        if(x->buffRef) {
            err = buffer_ref_notify(x->buffRef, s, msg, sender, data);
            if(buffer_ref_exists(x->buffRef) == 0) {
                object_free(x->buffRef);
                x->buffRef = NULL;
            }
        }
    }*/
    
    return err;
}

t_max_err sofa_hrir_attrSetSofa(t_sofa_hrir *x, t_object *attr, long argc, t_atom *argv) {
    if(argc) {
        t_symbol* a = atom_getsym(argv);
        sofa_hrir_setSofa(x, a);
    }
    return 0;
}

t_max_err sofa_hrir_attrGetSofa(t_sofa_hrir *x, t_object *attr, long *argc, t_atom **argv) {
    char alloc;
    atom_alloc(argc, argv, &alloc);
    atom_setsym(*argv, x->sofa_name);
    return 0;
}

void sofa_hrir_free(t_sofa_hrir *x) {
    if(x->sofa_name) {
        object_unsubscribe(APL_SOFA_NAMESPACE, x->sofa_name, APL_SOFA_CLASSNAME, x);
    }
    if(x->buffRef) {
        object_free(x->buffRef);
    }
    if(x->sourcePositions) {
        sysmem_freeptr(x->sourcePositions);
    }
    if(x->tree) {
        kd_free(x->tree);
    }
}

void *sofa_hrir_new(t_symbol *s, long argc, t_atom *argv) {
    t_sofa_hrir *x = NULL;
    t_symbol* sofa_name = NULL;
    t_symbol* buff_name = NULL;

    if(argc) {
        if(argv->a_type == A_SYM) {
            sofa_name = atom_getsym(argv);
        }
        else {
            object_error((t_object*)x, "sofa~ name must be a symbol");
            return x;
        }
        argv++;
    }
    
    if(argc > 1) {
        if(argv->a_type == A_SYM) {
            buff_name = atom_getsym(argv);
        }
        else {
            object_error((t_object*)x, "buffer~ name must be a symbol");
            return x;
        }
        argv++;
    }

    if(argc > 2) {
        object_warn((t_object*)x, "extra arguments for object");
    }

    if((x = (t_sofa_hrir *)object_alloc(sofa_hrir_class))) {
        x->sofa_ob = NULL;
        x->isBoundToSofa = false;
        if(sofa_name) {
            x->sofa_name = sofa_name;
            sofa_hrir_setSofa(x, sofa_name);
        }
        x->buffRef = NULL;
        x->buffName = buff_name;
        if(buff_name) {
            x->buffSet = sofa_hrir_registerBuffer(x, buff_name);
        }
        x->outlet_dump = outlet_new((t_object*)x, NULL);
    }
    return x;
}
