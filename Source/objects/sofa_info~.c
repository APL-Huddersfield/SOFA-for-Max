/**
	@file
	sofa.info~ - A SOFA importer for Max
	Dale Johnson - Dale.Johnson@hud.ac.uk

	@ingroup	examples
*/

#include "../dep/sofa_common.h"
#include "ext.h"

#include "ext_common.h"
#include "ext_globalsymbol.h"
#include "z_dsp.h"

typedef struct _sofa_info{
    t_object ob;
    t_sofa_max* sofa_ob;
    t_symbol* sofa_name;
    bool isBoundToSofa;

    void* outlet_dump;
    void* outlet_convention;
    void* outlet_sampleRate;
    void* outlet_N;
    void* outlet_E;
    void* outlet_R;
    void* outlet_M;
}t_sofa_info;

typedef enum _sofa_info_outlets {
    SOFA_INFO_M_OUTLET = 0,
    SOFA_INFO_R_OUTLET,
    SOFA_INFO_E_OUTLET,
    SOFA_INFO_N_OUTLET,
    SOFA_INFO_SAMPLERATE_OUTLET,
    SOFA_INFO_CONVENTION_OUTLET,
    SOFA_INFO_DUMP_OUTLET
}t_sofa_info_outlets;

void *sofa_info_new(t_symbol *s, long argc, t_atom *argv);
void sofa_info_free(t_sofa_info* x);

void sofa_info_output(t_sofa_info* x);
void sofa_info_setSofa(t_sofa_info* x, t_symbol* s);
void sofa_info_dumpAttributes(t_sofa_info* x);
void sofa_info_getPositions(t_sofa_info* x, t_symbol* s);

bool sofa_info_isSofaValid(t_sofa_info *x, t_symbol *s);
t_max_err sofa_info_notify(t_sofa_info *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
t_max_err sofa_info_attrSetSofa(t_sofa_info *x, t_object *attr, long argc, t_atom *argv);
t_max_err sofa_info_attrGetSofa(t_sofa_info *x, t_object *attr, long *argc, t_atom **argv);

void sofa_info_assist(t_sofa_info *x, void *b, long m, long a, char *s);

void *sofa_info_class;

void ext_main(void *r) {
	t_class *c;

	c = class_new("sofa.info~", (method)sofa_info_new, (method)sofa_info_free, (long)sizeof(t_sofa_info), 0L, A_GIMME, 0);

    class_addmethod(c, (method)sofa_info_output,            "bang", 0);
    class_addmethod(c, (method)sofa_info_setSofa,           "set",              A_SYM, 0);
    class_addmethod(c, (method)sofa_info_dumpAttributes,    "getattributes",    A_NOTHING, 0);
    class_addmethod(c, (method)sofa_info_getPositions,      "getpositions",     A_SYM, 0);

    class_addmethod(c, (method)sofa_info_notify,            "notify",           A_CANT, 0);
	class_addmethod(c, (method)sofa_info_assist,            "assist",           A_CANT, 0);

    CLASS_ATTR_SYM(c, "sofaobject", 0, t_sofa_info, sofa_name);
    CLASS_ATTR_ACCESSORS(c, "sofaobject", sofa_info_attrGetSofa, sofa_info_attrSetSofa);
    CLASS_ATTR_DEFAULTNAME_SAVE(c, "sofaobject", 0, "not set");
    CLASS_ATTR_LABEL(c, "sofaobject", 0, "sofa~ object");

	class_register(CLASS_BOX, c);
	sofa_info_class = c;
}

void sofa_info_assist(t_sofa_info *x, void *b, long m, long a, char *s) {
	if (m == ASSIST_INLET) {
        switch(a) {
            case 0:
                sprintf(s, "(anything) Message inlet");
                break;
        }
	}
	else {
        switch(a) {
            case SOFA_INFO_M_OUTLET:
                sprintf(s, "M / Number of measurements");
                break;
            case SOFA_INFO_R_OUTLET:
                sprintf(s, "R / Number of receivers");
                break;
            case SOFA_INFO_E_OUTLET:
                sprintf(s, "E / Number of emmitters");
                break;
            case SOFA_INFO_N_OUTLET:
                sprintf(s, "N / Measurement block size in samples");
                break;
            case SOFA_INFO_SAMPLERATE_OUTLET:
                sprintf(s, "(float) Sample rate");
                break;
            case SOFA_INFO_CONVENTION_OUTLET:
                sprintf(s, "(symbol) Convention");
                break;
            case SOFA_INFO_DUMP_OUTLET:
                sprintf(s, "dumpout");
                break;
        }
	}
}

void sofa_info_output(t_sofa_info* x) {
    if(!sofa_info_isSofaValid(x, gensym("bang"))) {
        return;
    }

    sofa_info_dumpAttributes(x);
    outlet_anything(x->outlet_convention, gensym(sofa_getConventionString(x->sofa_ob->sofa->convention)), 0, 0L);
    outlet_float(x->outlet_sampleRate, x->sofa_ob->sofa->sampleRate);
    outlet_int(x->outlet_N, x->sofa_ob->sofa->N);
    outlet_int(x->outlet_E, x->sofa_ob->sofa->E);
    outlet_int(x->outlet_R, x->sofa_ob->sofa->R);
    outlet_int(x->outlet_M, x->sofa_ob->sofa->M);
}

void sofa_info_setSofa(t_sofa_info* x, t_symbol* s) {
//    t_sofa_max* ref = (t_sofa_max*)globalsymbol_reference((t_object*)x, s->s_name, "sofa~");
//    if(ref != NULL) {
//        x->sofa_ob = ref;
//        object_subscribe(APL_SOFA_NAMESPACE, s, APL_SOFA_CLASSNAME, x);
//        x->isBoundToSofa = true;
//    }
    object_attr_setsym((t_object*)x, gensym("sofaobject"), s);
}

void sofa_info_dumpAttributes(t_sofa_info* x) {
    if(!sofa_info_isSofaValid(x, gensym("getattributes"))) {
        return;
    }

    t_symbol* mess = gensym("attribute");
    t_atom argv[2];
    t_symbol* name;
    t_symbol* value;
    long n = x->sofa_ob->sofa->attr.numAttributes;
    for(long i = 0; i < n; ++i) {
        name = gensym(x->sofa_ob->sofa->attr.names[i]);
        value = gensym(x->sofa_ob->sofa->attr.values[i]);
        atom_setsym(argv, name);
        atom_setsym(argv + 1, value);
        outlet_anything(x->outlet_dump, mess, 2, argv);
    }
}

void sofa_info_getPositions(t_sofa_info* x, t_symbol* s) {
    if(sofa_info_isSofaValid(x, gensym("getpositions"))) {
        sofa_getPositions(x->sofa_ob, x->outlet_dump, s);
    }
}

bool sofa_info_isSofaValid(t_sofa_info* x, t_symbol* mess) {
    if(!x->isBoundToSofa) {
        object_error((t_object*)x, "%s: no sofa~ object set", mess->s_name);
        return false;
    }
    if(!isSofaFileOpen((t_object*)x, x->sofa_ob, mess)) {
        return false;
    }
    return true;
}

t_max_err sofa_info_notify(t_sofa_info *x, t_symbol *s, t_symbol *msg, void *sender, void *data) {
    t_max_err err = MAX_ERR_NONE;

    if(msg == gensym("globalsymbol_binding")) {
        if(x->sofa_name) {
            if(object_findregistered(APL_SOFA_NAMESPACE, x->sofa_name)) {
                sofa_info_setSofa(x, x->sofa_name);
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
    return err;
}

t_max_err sofa_info_attrSetSofa(t_sofa_info *x, t_object *attr, long argc, t_atom *argv) {
//    if(argc) {
//        t_symbol* a = atom_getsym(argv);
//        sofa_info_setSofa(x, a);
//    }
    
    if (argc) {
        t_symbol* a = atom_getsym(argv);
        x->sofa_name = a;
        t_sofa_max* ref = (t_sofa_max*)globalsymbol_reference((t_object*)x, a->s_name, "sofa~");
        if(ref != NULL) {
            x->sofa_ob = ref;
            object_subscribe(APL_SOFA_NAMESPACE, a, APL_SOFA_CLASSNAME, x);
            x->isBoundToSofa = true;
        }
    }
    
    return 0;
}

t_max_err sofa_info_attrGetSofa(t_sofa_info *x, t_object *attr, long *argc, t_atom **argv) {
    char alloc;
    atom_alloc(argc, argv, &alloc);
    atom_setsym(*argv, x->sofa_name);
    return 0;
}

void sofa_info_free(t_sofa_info *x) {
    if(x->sofa_name) {
        object_unsubscribe(APL_SOFA_NAMESPACE, x->sofa_name, APL_SOFA_CLASSNAME, x);
    }
}

void *sofa_info_new(t_symbol *s, long argc, t_atom *argv) {
    t_sofa_info *x = NULL;
    t_symbol* sofa_name = NULL;

    if(argc >= 1) {
        if(atomIsASymbol(argv)) {
            sofa_name = atom_getsym(argv);
        }
        else {
            object_error((t_object*)x, "name must be a symbol");
            return x;
        }
    }

    if(argc > 1) {
        object_warn((t_object*)x, "extra arguments for object");
    }

    if((x = (t_sofa_info *)object_alloc(sofa_info_class))) {
        x->sofa_ob = NULL;
        x->isBoundToSofa = false;
        if(sofa_name) {
            x->sofa_name = sofa_name;
            sofa_info_setSofa(x, sofa_name);
        }

        x->outlet_dump = outlet_new((t_object*)x, NULL);
        x->outlet_convention = outlet_new((t_object*)x, NULL);
        x->outlet_sampleRate = floatout((t_object*)x);
        x->outlet_N = intout((t_object*)x);
        x->outlet_E = intout((t_object*)x);
        x->outlet_R = intout((t_object*)x);
        x->outlet_M = intout((t_object*)x);
    }

    return (x);
}
