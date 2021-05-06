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

typedef struct _sofa_poke {
    t_object ob;
    t_sofa_max* sofa_ob;
    t_symbol* sofa_name;
    bool isBoundToSofa;

    t_buffer_ref* buffRef;
    t_symbol* buffName;
    bool buffSet;

    void* outlet_dump;
}t_sofa_poke;

typedef enum _sofa_poke_outlets {
    BANG_OUTLET = 0,
    DUMP_OUTLET
}t_sofa_poke_outlets;

void *sofa_poke_new(t_symbol* s, long argc, t_atom* argv);
void sofa_poke_free(t_sofa_poke* x);

void sofa_poke_setSofa(t_sofa_poke* x, t_symbol* s);
void sofa_poke_setBuffer(t_sofa_poke* x, t_symbol* s);
bool sofa_poke_registerBuffer(t_sofa_poke* x, t_symbol *s);

void sofa_poke_putMeasurement(t_sofa_poke* x, t_symbol* s, long argc, t_atom* argv);
void sofa_poke_setPosition(t_sofa_poke* x, t_symbol* s, long argc, t_atom* argv);

t_max_err sofa_poke_notify(t_sofa_poke* x, t_symbol* s, t_symbol* msg, void* sender, void* data);
t_max_err sofa_poke_attrSetSofa(t_sofa_poke* x, t_object* attr, long argc, t_atom* argv);
t_max_err sofa_poke_attrGetSofa(t_sofa_poke* x, t_object* attr, long* argc, t_atom** argv);

void sofa_poke_assist(t_sofa_poke* x, void* b, long m, long a, char* s);

void *sofa_poke_class;


void ext_main(void *r) {
	t_class *c;

	c = class_new("sofa.poke~", (method)sofa_poke_new, (method)sofa_poke_free, (long)sizeof(t_sofa_poke), 0L, A_GIMME, 0);

    class_addmethod(c, (method)sofa_poke_setSofa,           "set",              A_SYM,      0);
    class_addmethod(c, (method)sofa_poke_setBuffer,         "setbuffer",        A_SYM,      0);
    class_addmethod(c, (method)sofa_poke_putMeasurement,    "put",              A_GIMME,    0);
    class_addmethod(c, (method)sofa_poke_setPosition,       "position",         A_GIMME,    0);

    class_addmethod(c, (method)sofa_poke_notify,            "notify",           A_CANT,     0);
	class_addmethod(c, (method)sofa_poke_assist,            "assist",           A_CANT,     0);

    CLASS_ATTR_SYM(c, "sofaobject", 0, t_sofa_poke, sofa_name);
    CLASS_ATTR_ACCESSORS(c, "sofaobject", sofa_poke_attrGetSofa, sofa_poke_attrSetSofa);
    CLASS_ATTR_SELFSAVE(c, "sofaobject", 0);
    CLASS_ATTR_LABEL(c, "sofaobject", 0, "sofa~ object");

	class_register(CLASS_BOX, c);
	sofa_poke_class = c;
}

void sofa_poke_setSofa(t_sofa_poke* x, t_symbol* s) {
    t_sofa_max* ref = (t_sofa_max*)globalsymbol_reference((t_object*)x, s->s_name, "sofa~");
    if(ref != NULL) {
        x->sofa_ob = ref;
        object_subscribe(APL_SOFA_NAMESPACE, s, APL_SOFA_CLASSNAME, x);
        x->isBoundToSofa = true;
    }
}

void sofa_poke_setBuffer(t_sofa_poke* x, t_symbol* s) {
    if(!sofa_poke_registerBuffer(x, s)) {
        object_error((t_object*)x, "setbuffer: buffer \"%s\" does not exist", s->s_name);
    }
}

bool sofa_poke_registerBuffer(t_sofa_poke* x, t_symbol* s) {
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

void sofa_poke_assist(t_sofa_poke *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
        switch(a) {
            case 0:
                sprintf(s, "(anything) Message inlet");
                break;
        }
	}
	else {
        switch(a) {
            case DUMP_OUTLET:
                sprintf(s, "(anything) Dump outlet");
                break;
        }
	}
}

t_max_err sofa_poke_notify(t_sofa_poke *x, t_symbol *s, t_symbol *msg, void *sender, void *data) {
    t_max_err err = MAX_ERR_NONE;

    if(msg == gensym("globalsymbol_binding")) {
        if(x->sofa_name) {
            if(object_findregistered(APL_SOFA_NAMESPACE, x->sofa_name)) {
                sofa_poke_setSofa(x, x->sofa_name);
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
    
    t_symbol* buffName = (t_symbol*)object_method(sender, gensym("getname"));
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
    
    /*if(x->buffRef) {
        buffer_ref_notify(x->buffRef, s, msg, sender, data);
        if(buffer_ref_exists(x->buffRef) == 0) {
            object_free(x->buffRef);
            x->buffRef = NULL;
        }
    }*/
    
    return err;
}

t_max_err sofa_poke_attrSetSofa(t_sofa_poke *x, t_object *attr, long argc, t_atom *argv) {
    if(argc) {
        t_symbol* a = atom_getsym(argv);
        sofa_poke_setSofa(x, a);
    }
    return 0;
}

t_max_err sofa_poke_attrGetSofa(t_sofa_poke *x, t_object *attr, long *argc, t_atom **argv) {
    char alloc;
    atom_alloc(argc, argv, &alloc);
    atom_setsym(*argv, x->sofa_name);
    return 0;
}

void sofa_poke_putMeasurement(t_sofa_poke* x, t_symbol* s, long argc, t_atom* argv) {
    if(*x->sofa_ob->fileLoaded == false) {
        object_error((t_object*)x, "%s: no SOFA data has been loaded / initialised", s->s_name);
        return;
    }
    
    long m = 0;
    long r = 0;
    long e = 0;
    t_symbol* optionalBufferName;
    bool optionalBufferGiven = false;
    t_sofaConvention convention = x->sofa_ob->sofa->convention;
    
    if(argc < 2) {
        object_error((t_object*)x, "%s: not enough arguments for adding measurement data", s->s_name);
        return;
    }
    
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: measurement argument should be an integer", s->s_name);
        return;
    }
    m = atom_getlong(argv);
    argv++;
    
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: receiver argument should be an integer", s->s_name);
        return;
    }
    r = atom_getlong(argv);
    argv++;
    
    if(argc > 2) {
        if(convention == SOFA_GENERAL_FIRE || convention == SOFA_MULTISPEAKER_BRIR) {
            if(!atomIsANumber(argv)) {
                object_error((t_object*)x, "%s: for %s convention, the emitter argument should be an integer", s->s_name, sofa_getConventionString(convention));
                return;
            }
            e = atom_getlong(argv);
            argv++;
        }
        if(!atomIsASymbol(argv)) {
            object_error((t_object*)x, "%s: optional buffer~ name must be a symbol", s->s_name);
            return;
        }
        else {
            optionalBufferName = atom_getsym(argv);
            optionalBufferGiven = true;
        }
    }
    
    if(m < 0 || m >= x->sofa_ob->sofa->M) {
        object_error((t_object*)x, "%s: measurement number out of range", s->s_name);
        return;
    }
    if(r < 0 || r >= x->sofa_ob->sofa->R) {
        object_error((t_object*)x, "%s: receiver number out of range", s->s_name);
        return;
    }
    if(e < 0 || e >= x->sofa_ob->sofa->E) {
        object_error((t_object*)x, "%s: emitter number out of range", s->s_name);
        return;
    }
    
    // Buffer Operations
    
    t_buffer_ref* buffRef = x->buffRef;
    t_buffer_obj* buffObj;
    
    if(optionalBufferGiven) {
        buffRef = buffer_ref_new((t_object*)x, optionalBufferName);
        if(buffer_ref_exists(buffRef) == 0) {
            object_error((t_object*)x, "%s: source buffer \"%s\" does not exist", s->s_name, optionalBufferName->s_name);
            object_free(buffRef);
            return;
        }
    }
    else if(buffRef == NULL) {
        object_error((t_object*)x, "%s: no source buffer has been set", s->s_name);
        return;
    }
    
    buffObj = buffer_ref_getobject(buffRef);
    
    long numBuffChans = buffer_getchannelcount(buffObj);
    long numBuffFrames = buffer_getframecount(buffObj);
    
    if(numBuffFrames >= x->sofa_ob->sofa->N) {
        object_warn((t_object*)x, "%s: source buffer is too long and will be cropped to fit", s->s_name);
    }
    
    // Do copy
    
    float* buffData = buffer_locksamples(buffObj);
    double* dataIR = (double*)sysmem_newptr(sizeof(double) * x->sofa_ob->sofa->N);
    long buffIndex = 0;
    long N = x->sofa_ob->sofa->N;
    if(buffData) {
        for(long i = 0; i < N; ++i) {
            buffIndex = (i * numBuffChans);
            if(i < numBuffFrames) {
                dataIR[i] = buffData[buffIndex];
            }
            else {
                dataIR[i] = 0.0;
            }
        }
        buffer_unlocksamples(buffObj);
    }
    else {
        buffer_unlocksamples(buffObj);
    }
    
    if(optionalBufferGiven && buffRef) {
        object_free(buffRef);
    }
    
    if(convention == SOFA_GENERAL_FIRE || convention == SOFA_MULTISPEAKER_BRIR) {
        csofa_setMREDataBlock(x->sofa_ob->sofa, m, r, e, dataIR, x->sofa_ob->sofa->N);
    }
    else {
        csofa_setMRDataBlock(x->sofa_ob->sofa, m, r, dataIR, x->sofa_ob->sofa->N);
    }
    sysmem_freeptr(dataIR);
}

void sofa_poke_setPosition(t_sofa_poke* x, t_symbol* s, long argc, t_atom* argv) {
    t_symbol* var;
    long id = 0;
    if(*x->sofa_ob->fileLoaded == false) {
        object_error((t_object*)x, "%s: no SOFA data has been loaded / initialised", s->s_name);
        return;
    }
    
    /* Args list: <position type>, <position ID>, <component 0>, <component 1>, <component 2> */
    long minNumArgs = 2;
    switch(x->sofa_ob->sofa->convention) {
        case SOFA_SIMPLE_FREE_FIELD_HRIR:
            minNumArgs = 4;
            break;
        default:
            break;
    }
    
    if (argc < minNumArgs) {
        object_error((t_object*)x,
                     "%s: not enough arguments to set position. Expected at least %ld arguments",
                     s->s_name, minNumArgs);
        return;
    }

    /* Get and validate position var type */
    long varTypeId = 0;
    t_sofaVarType varType = SOFA_UNKNOWN_TYPE;
    if (argv->a_type == A_SYM) {
        var = atom_getsym(argv);
        varType = sofa_hashVarTypeFromName(var);
    }
    else if (argv->a_type == A_LONG) {
        varTypeId = atom_getlong(argv);
        if (varTypeId < 0) {
            varTypeId = 0;
        }
        if (varTypeId < (long)SOFA_NUM_VAR_TYPES) {
            varType = (t_sofaConvention)varTypeId;
        }
    }
    
    if (varType == SOFA_UNKNOWN_TYPE) {
        object_error((t_object*)x, "%s: requested invalid position type", var);
        return;
    }
    argv++;

    /* Get and validate measurment ID*/
    if (argv->a_type != A_LONG) {
        return;
    }
    id = atom_getlong(argv);

    if ((t_sofaVarType)varType == SOFA_VAR_SOURCE) {
        if (id >= x->sofa_ob->sofa->M) {
            object_error((t_object*)x, "%s: requested measurement ID is out of range", s->s_name);
            return;
        }
    }
    argv++;
    
    /* Get co-ordinates */
    // TODO: Extract and validate components from args.
    double components[3] = {0.0, 0.0, 1.0};
    long numComponentArgs = argc - 2;
    long minNumComponents = 2;
    
    for (long i = 0; i < numComponentArgs; ++i) {
        if ((argv + i)->a_type != A_LONG) {
            object_error((t_object*)x, "%s: co-ordinate component %ld is not a number",
                         s->s_name, i);
            return;
        }
    }

    if (x->sofa_ob->sofa->convention == SOFA_SIMPLE_FREE_FIELD_HRIR) {
        minNumComponents = 2;
        if (numComponentArgs < minNumComponents) {
            
        }
    }
    
    /* Construct point based on convention type */
    t_point p;
    switch (x->sofa_ob->sofa->convention) {
        case SOFA_SIMPLE_FREE_FIELD_HRIR:
            p = makeSphericalPoint(id, components[0], components[1]);
            break;
        default:
            p = make2DCartesianPoint(id, 0.0, 0.0);
    }
    
    /* Set the requested measurement position */
    if (csofa_setPosition(x->sofa_ob->sofa, (t_sofaVarType)varType, id, &p) == false) {
        object_error((t_object*)x, "%s: error setting position");
    }
}

void sofa_poke_free(t_sofa_poke *x) {
    if(x->sofa_name) {
        object_unsubscribe(APL_SOFA_NAMESPACE, x->sofa_name, APL_SOFA_CLASSNAME, x);
    }
    if(x->buffRef) {
        object_free(x->buffRef);
    }
}

void *sofa_poke_new(t_symbol *s, long argc, t_atom *argv) {
    t_sofa_poke *x = NULL;
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

    if((x = (t_sofa_poke *)object_alloc(sofa_poke_class))) {
        x->sofa_ob = NULL;
        x->isBoundToSofa = false;
        if(sofa_name) {
            x->sofa_name = sofa_name;
            sofa_poke_setSofa(x, sofa_name);
        }
        x->buffRef = NULL;
        x->buffName = buff_name;
        if(buff_name) {
            x->buffSet = sofa_poke_registerBuffer(x, buff_name);
        }
        x->outlet_dump = outlet_new((t_object*)x, NULL);
    }
    return x;
}
