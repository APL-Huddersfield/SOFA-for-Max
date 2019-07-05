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

typedef struct _sofa_srdrir {
    t_object ob;
    t_sofa_max* sofa_ob;
    t_symbol* sofa_name;
    bool isBoundToSofa;
    
    double* dataIR;

    t_buffer_ref* buffRef;
    t_symbol* buffName;
    bool buffSet;

    void* outlet_dump;
    void* out_bang;
}t_sofa_srdrir;

typedef enum _sofa_srdrir_outlets {
    BANG_OUTLET = 0,
    DUMP_OUTLET
}t_sofa_srdrir_outlets;

void *sofa_srdrir_new(t_symbol* s, long argc, t_atom* argv);
void sofa_srdrir_free(t_sofa_srdrir* x);

void sofa_srdrir_setSofa(t_sofa_srdrir* x, t_symbol* s);
void sofa_srdrir_setBuffer(t_sofa_srdrir* x, t_symbol* s);
bool sofa_srdrir_registerBuffer(t_sofa_srdrir* x, t_symbol *s);

void sofa_srdrir_getBlock(t_sofa_srdrir* x, t_symbol* s, long argc, t_atom* argv);
void sofa_srdrir_getMR(t_sofa_srdrir* x, t_symbol* s, long argc, t_atom* argv);
void sofa_srdrir_get(t_sofa_srdrir* x, t_symbol* s, long block, t_symbol* bufferName, long channel);

void sofa_srdrir_getSize(t_sofa_srdrir* x);
void sofa_srdrir_getSizeSamps(t_sofa_srdrir* x);

bool sofa_srdrir_isSofaValid(t_sofa_srdrir* x, t_symbol* s);
t_max_err sofa_srdrir_notify(t_sofa_srdrir* x, t_symbol* s, t_symbol* msg, void* sender, void* data);
t_max_err sofa_srdrir_attrSetSofa(t_sofa_srdrir* x, t_object* attr, long argc, t_atom* argv);
t_max_err sofa_srdrir_attrGetSofa(t_sofa_srdrir* x, t_object* attr, long* argc, t_atom** argv);

void sofa_srdrir_assist(t_sofa_srdrir* x, void* b, long m, long a, char* s);

void *sofa_srdrir_class;


void ext_main(void *r) {
	t_class *c;

	c = class_new("sofa.srdrir~", (method)sofa_srdrir_new, (method)sofa_srdrir_free, (long)sizeof(t_sofa_srdrir), 0L, A_GIMME, 0);

    class_addmethod(c, (method)sofa_srdrir_getSize,        "getsize",      A_NOTHING, 0);
    class_addmethod(c, (method)sofa_srdrir_getSizeSamps,   "getsizesamps", A_NOTHING, 0);

    class_addmethod(c, (method)sofa_srdrir_setSofa,        "set",          A_SYM, 0);
    class_addmethod(c, (method)sofa_srdrir_setBuffer,      "setbuffer",    A_SYM, 0);
    class_addmethod(c, (method)sofa_srdrir_getBlock,       "getblock",     A_GIMME, 0);
    class_addmethod(c, (method)sofa_srdrir_getMR,          "get",          A_GIMME, 0);

    class_addmethod(c, (method)sofa_srdrir_notify,         "notify",       A_CANT, 0);
	class_addmethod(c, (method)sofa_srdrir_assist,         "assist",       A_CANT, 0);

    CLASS_ATTR_SYM(c, "sofaobject", 0, t_sofa_srdrir, sofa_name);
    CLASS_ATTR_ACCESSORS(c, "sofaobject", sofa_srdrir_attrGetSofa, sofa_srdrir_attrSetSofa);
    CLASS_ATTR_SELFSAVE(c, "sofaobject", 0);
    CLASS_ATTR_LABEL(c, "sofaobject", 0, "sofa~ object");

	class_register(CLASS_BOX, c);
	sofa_srdrir_class = c;
}

void sofa_srdrir_setSofa(t_sofa_srdrir* x, t_symbol* s) {
    t_sofa_max* ref = (t_sofa_max*)globalsymbol_reference((t_object*)x, s->s_name, "sofa~");
    if(ref != NULL) {
        x->sofa_ob = ref;
        object_subscribe(APL_SOFA_NAMESPACE, s, APL_SOFA_CLASSNAME, x);
        x->isBoundToSofa = true;
    }
}

void sofa_srdrir_setBuffer(t_sofa_srdrir* x, t_symbol* s) {
    if(!sofa_srdrir_registerBuffer(x, s)) {
        object_error((t_object*)x, "setbuffer: buffer \"%s\" does not exist", s->s_name);
    }
}

bool sofa_srdrir_registerBuffer(t_sofa_srdrir* x, t_symbol* s) {
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

void sofa_srdrir_getSize(t_sofa_srdrir* x) {
    if(x->sofa_ob) {
        if(x->sofa_ob->sofa->N) {
            t_atom argv;
            atom_setfloat(&argv, x->sofa_ob->sofa->N / sys_getsr() * 1000.f);
            outlet_anything(x->outlet_dump, gensym("size"), 1, &argv);
        }
    }
}

void sofa_srdrir_getSizeSamps(t_sofa_srdrir* x) {
    if(x->sofa_ob) {
        if(x->sofa_ob->sofa->N) {
            t_atom argv;
            atom_setlong(&argv, x->sofa_ob->sofa->N);
            outlet_anything(x->outlet_dump, gensym("sizesamps"), 1, &argv);
        }
    }
}

void sofa_srdrir_assist(t_sofa_srdrir *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
        switch(a) {
            case 0:
                sprintf(s, "(anything) Message inlet");
                break;
        }
	}
    else {
        switch(a) {
            case BANG_OUTLET:
                sprintf(s, "(bang) Extraction successful");
                break;
            case DUMP_OUTLET:
                sprintf(s, "(anything) Dump outlet");
                break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HRTF Extraction
////////////////////////////////////////////////////////////////////////////////////////////////////

void sofa_srdrir_getBlock(t_sofa_srdrir* x, t_symbol* s, long argc, t_atom* argv) {
    long block = 0;
    t_symbol* bufferName = NULL;
    long bufferChannel = -1;
    
    if(argc < 1) {
        object_error((t_object*)x, "%s: not enough arguments, expecting at least 1", s->s_name);
        return;
    }
    if(!sofa_srdrir_isSofaValid(x, s)) {
        return;
    }
    
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: first argument should be a number", s->s_name);
        return;
    }
    block = atom_getlong(argv);
    argv++;
    
    if(block < 0 || block > x->sofa_ob->sofa->numBlocks - 1) {
        return;
    }
    
    if(argc > 1) {
        if(!atomIsASymbol(argv)) {
            object_error((t_object*)x, "%s: second argument should be a symbol", s->s_name);
            return;
        }
        bufferName = atom_getsym(argv);
        argv++;
    }
    
    if(argc > 2) {
        if(!atomIsANumber(argv)) {
            object_error((t_object*)x, "%s: third argument should be a number", s->s_name);
            return;
        }
        bufferChannel = atom_getlong(argv);
        argv++;
    }
    
    if(argc > 3) {
        object_warn((t_object*)x, "extra arguments for message \"%s\"", s->s_name);
    }
    
    sofa_srdrir_get(x, s, block, bufferName, bufferChannel);
}

void sofa_srdrir_getMR(t_sofa_srdrir* x, t_symbol* s, long argc, t_atom* argv) {
    long m = 0;
    long r = 0;
    long block = 0;
    t_symbol* bufferName = NULL;
    long bufferChannel = -1;
    
    if(argc < 2) {
        object_error((t_object*)x, "%s: not enough arguments, expecting at least 2", s->s_name);
        return;
    }
    if(!sofa_srdrir_isSofaValid(x, s)) {
        return;
    }
    
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: first argument should be a number", s->s_name);
        return;
    }
    m = atom_getlong(argv);
    argv++;
    
    if(m < 0 || m > x->sofa_ob->sofa->M - 1) {
        return;
    }
    
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: second argument should be a number", s->s_name);
        return;
    }
    r = atom_getlong(argv);
    argv++;
    
    if(r < 0 || r > x->sofa_ob->sofa->R - 1) {
        return;
    }
    
    block = m * x->sofa_ob->sofa->R + r;
    
    if(argc > 2) {
        if(!atomIsASymbol(argv)) {
            object_error((t_object*)x, "%s: third argument should be a symbol", s->s_name);
            return;
        }
        bufferName = atom_getsym(argv);
        argv++;
    }
    
    
    if(argc > 3) {
        if(!atomIsANumber(argv)) {
            object_error((t_object*)x, "%s: fourth argument should be a number", s->s_name);
            return;
        }
        bufferChannel = atom_getlong(argv);
        argv++;
    }
    
    if(argc > 4) {
        object_warn((t_object*)x, "extra arguments for message \"%s\"", s->s_name);
    }
    
    sofa_srdrir_get(x, s, block, bufferName, bufferChannel);
}

void sofa_srdrir_get(t_sofa_srdrir* x, t_symbol* s, long block, t_symbol* bufferName, long channel) {
    // Buffer Operations
    
    t_buffer_ref* buffRef;// = x->buffRef;
    t_buffer_obj* buffObj;
    bool optionalBufferIsGiven = bufferName != NULL ? true : false;

    if(optionalBufferIsGiven) {
        buffRef = buffer_ref_new((t_object*)x, bufferName);
        if(buffer_ref_exists(buffRef) == 0) {
            object_error((t_object*)x, "%s: destination buffer \"%s\" does not exist", s->s_name, bufferName->s_name);
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
            x->buffSet = sofa_srdrir_registerBuffer(x, x->buffName);
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
    long buffChanOffset = 0;

    if(channel > -1) {
        if(channel > (numBuffChans - 1)) {
            if(optionalBufferIsGiven && buffRef) {
                object_free(buffRef);
            }
            return;
        }
        buffChanOffset = channel;
    }

    if(numBuffFrames < x->sofa_ob->sofa->N) {
        object_error((t_object*)x, "%s: destination buffer is too short", s->s_name);
        if(optionalBufferIsGiven && buffRef) {
            object_free(buffRef);
        }
        return;
    }

    // Do extraction
    double* d = csofa_getDataIR(x->sofa_ob->sofa, block);
    if(d == NULL) {
        object_error((t_object*)x, "%s: data extraction was unsuccesful", s->s_name);
        if(optionalBufferIsGiven && buffRef) {
                object_free(buffRef);
        }
        return;
    }

    float* buffData = buffer_locksamples(buffObj);
    long buffIndex = 0;
    long N = x->sofa_ob->sofa->N;
    if(buffData) {
        for(long i = 0; i < N; ++i) {
            buffIndex = (i * numBuffChans) + buffChanOffset;
            buffData[buffIndex] = d[i];
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
    
    outlet_bang(x->out_bang);
}

bool sofa_srdrir_isSofaValid(t_sofa_srdrir* x, t_symbol* mess) {
    if(!x->isBoundToSofa) {
        object_error((t_object*)x, "%s: no sofa~ object set", mess->s_name);
        return false;
    }
    if(!isSofaFileOpen((t_object*)x, x->sofa_ob, mess)) {
        return false;
    }
    if(x->sofa_ob->sofa->convention != SOFA_SINGLE_ROOM_DRIR) {
        object_error((t_object*)x, "%s: incompatible convention. sofa.srdrir~ expects a SingeRoomDRIR", mess->s_name);
        return false;
    }
    return true;
}

t_max_err sofa_srdrir_notify(t_sofa_srdrir *x, t_symbol *s, t_symbol *msg, void *sender, void *data) {
    t_max_err err = MAX_ERR_NONE;
    t_symbol* buffName = (t_symbol*)object_method(sender, gensym("getname"));

    if(msg == gensym("globalsymbol_binding")) {
        if(x->sofa_name) {
            if(object_findregistered(APL_SOFA_NAMESPACE, x->sofa_name)) {
                sofa_srdrir_setSofa(x, x->sofa_name);
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

t_max_err sofa_srdrir_attrSetSofa(t_sofa_srdrir *x, t_object *attr, long argc, t_atom *argv) {
    if(argc) {
        t_symbol* a = atom_getsym(argv);
        sofa_srdrir_setSofa(x, a);
    }
    return 0;
}

t_max_err sofa_srdrir_attrGetSofa(t_sofa_srdrir *x, t_object *attr, long *argc, t_atom **argv) {
    char alloc;
    atom_alloc(argc, argv, &alloc);
    atom_setsym(*argv, x->sofa_name);
    return 0;
}

void sofa_srdrir_free(t_sofa_srdrir *x) {
    if(x->sofa_name) {
        object_unsubscribe(APL_SOFA_NAMESPACE, x->sofa_name, APL_SOFA_CLASSNAME, x);
    }
    if(x->buffRef) {
        object_free(x->buffRef);
    }
}

void *sofa_srdrir_new(t_symbol *s, long argc, t_atom *argv) {
    t_sofa_srdrir *x = NULL;
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

    if((x = (t_sofa_srdrir *)object_alloc(sofa_srdrir_class))) {
        x->sofa_ob = NULL;
        x->isBoundToSofa = false;
        if(sofa_name) {
            x->sofa_name = sofa_name;
            sofa_srdrir_setSofa(x, sofa_name);
        }
        x->buffRef = NULL;
        x->buffName = buff_name;
        if(buff_name) {
            x->buffSet = sofa_srdrir_registerBuffer(x, buff_name);
        }
        x->outlet_dump = outlet_new((t_object*)x, NULL);
        x->out_bang = bangout((t_object*)x);
    }
    return x;
}
