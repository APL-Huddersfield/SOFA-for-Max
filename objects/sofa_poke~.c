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
    
    double* dataIR;

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

void sofa_poke_write(t_sofa_poke* x, t_symbol* s);

t_max_err sofa_poke_notify(t_sofa_poke* x, t_symbol* s, t_symbol* msg, void* sender, void* data);
t_max_err sofa_poke_attrSetSofa(t_sofa_poke* x, t_object* attr, long argc, t_atom* argv);
t_max_err sofa_poke_attrGetSofa(t_sofa_poke* x, t_object* attr, long* argc, t_atom** argv);

void sofa_poke_assist(t_sofa_poke* x, void* b, long m, long a, char* s);

void *sofa_poke_class;


void ext_main(void *r) {
	t_class *c;

	c = class_new("sofa.poke~", (method)sofa_poke_new, (method)sofa_poke_free, (long)sizeof(t_sofa_poke), 0L, A_GIMME, 0);

    class_addmethod(c, (method)sofa_poke_setSofa,       "set",          A_SYM,      0);
    class_addmethod(c, (method)sofa_poke_setBuffer,     "setbuffer",    A_SYM,      0);
    
    class_addmethod(c, (method)sofa_poke_write,         "write",        A_SYM,      0);

    class_addmethod(c, (method)sofa_poke_notify,        "notify",       A_CANT,     0);
	class_addmethod(c, (method)sofa_poke_assist,        "assist",       A_CANT,     0);

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

void sofa_poke_write(t_sofa_poke* x, t_symbol* s) {
    
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
