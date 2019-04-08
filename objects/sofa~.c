/**
	@file
	sofa~ - A SOFA importer for Max
	Dale Johnson - Dale.Johnson@hud.ac.uk

	@ingroup	examples
*/

#include "../dep/sofa_common.h"
#include "ext.h"
#include "ext_buffer.h"
#include "ext_common.h"
#include "ext_globalsymbol.h"

void *sofa_max_new(t_symbol *s, long argc, t_atom *argv);
void sofa_max_free(t_sofa_max *x);
void sofa_max_read(t_sofa_max *x, t_symbol *s);
void sofa_max_doRead(t_sofa_max* x, t_symbol* s);
void sofa_max_open(t_sofa_max* x, char* filename, short path);

void sofa_max_write(t_sofa_max* x, t_symbol* s);
void sofa_max_doWrite(t_sofa_max* x, t_symbol* s);

// Data manipulation methods
void sofa_max_create(t_sofa_max* x, t_symbol* s, long argc, t_atom* argv);

// Measurement acquisition methods
t_buffer_ref* sofa_max_getNewBuffer(t_sofa_max* x, t_symbol* bufferName);
void sofa_max_getPositions(t_sofa_max* x, t_symbol* s);
void sofa_max_getDimension(t_sofa_max* x, t_symbol* s, long argc, t_atom *argv);
void sofa_max_getSize(t_sofa_max* x);
void sofa_max_getName(t_sofa_max* x);
void sofa_max_get(t_sofa_max* x, t_symbol* s, long argc, t_atom *argv);

bool sofa_max_isFileLoaded(t_sofa_max* x, t_symbol* s);

void sofa_max_notify(t_sofa_max *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void sofa_max_assist(t_sofa_max *x, void *b, long m, long a, char *s);

void *sofa_max_class;

void ext_main(void *r) {
	t_class *c;

	c = class_new("sofa~", (method)sofa_max_new, (method)sofa_max_free, (long)sizeof(t_sofa_max), 0L, A_GIMME, 0);

    class_addmethod(c, (method)sofa_max_read,           "read",         A_DEFSYM, 0);
    class_addmethod(c, (method)sofa_max_write,          "write",        A_DEFSYM, 0);
    class_addmethod(c, (method)sofa_max_create,         "new",          A_GIMME, 0);
    
    class_addmethod(c, (method)sofa_max_getPositions,   "getpositions", A_SYM, 0);
    class_addmethod(c, (method)sofa_max_getDimension,   "getdimension", A_GIMME, 0);
    class_addmethod(c, (method)sofa_max_getSize,        "getsizesamps", A_NOTHING, 0);
    class_addmethod(c, (method)sofa_max_getName,        "getname",      A_NOTHING, 0);
    class_addmethod(c, (method)sofa_max_get,            "get",          A_GIMME, 0);
    
	class_addmethod(c, (method)sofa_max_assist,         "assist",       A_CANT, 0);

	class_register(CLASS_BOX, c);
	sofa_max_class = c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


void sofa_max_read(t_sofa_max* x, t_symbol* s) {
    defer(x, (method)sofa_max_doRead, s, 0, NULL);
}

void sofa_max_doRead(t_sofa_max* x, t_symbol* s) {
    t_fourcc filetype = 'NULL';
    t_fourcc outtype;
    short path;
    char filename[MAX_PATH_CHARS];

    if(s == gensym("")) {
        if(open_dialog(filename, &path, &outtype, &filetype, 0)) {
            return;
        }
    }
    else {
        strcpy(filename, s->s_name);
        object_post((t_object*)x, "%s", filename);

        if(locatefile_extended(filename, &path, &outtype, &filetype, 0)) {
            object_error((t_object*)x, "%s: file not found", s->s_name);
            return;
        }
    }

    unsigned long filenameLength = strlen(filename);

    if(strcmp(filename + (filenameLength - 4), "sofa")) {
        object_error((t_object*)x, "%s: %s is not a valid .sofa file", s->s_name, filename);
        return;
    }

    sofa_max_open(x, filename, path);
}

void sofa_max_open(t_sofa_max* x, char* filename, short path) {
    t_filehandle fh;
    if(path_opensysfile(filename, path, &fh, READ_PERM)) {
        object_error((t_object*)x, "error opening %s", filename);
        return;
    }
    sysfile_close(fh);
    char fullpath[MAX_PATH_CHARS];
    path_toabsolutesystempath(path, filename, fullpath);
    if(*x->fileLoaded) {
        csofa_destroySofa(x->sofa);
        *x->fileLoaded = false;
    }

    critical_enter(0);
    *x->sofa = csofa_openFile(fullpath);
    critical_exit(0);
    
    *x->fileLoaded = true;
    object_notify((t_object*)x, gensym("sofaread"), 0L);
    outlet_bang(x->outlet_finishedLoading);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void sofa_max_write(t_sofa_max* x, t_symbol* s) {
    defer(x, (method)sofa_max_doWrite, s, 0, NULL);
}

void sofa_max_doWrite(t_sofa_max* x, t_symbol* s) {
    t_fourcc filetype = 'NULL';
    t_fourcc outtype;
    char filename[MAX_PATH_CHARS];
    char fullpath[MAX_PATH_CHARS];
    short path;
    
    if(s == gensym("")) {
        if(saveasdialog_extended(filename, &path, &outtype, &filetype, 1)) {
            return;
        }
    }
    else {
        strcpy(filename, s->s_name);
        path = path_getdefault();
    }
    path_toabsolutesystempath(path, filename, fullpath);
    critical_enter(0);
    csofa_writeFile(x->sofa, fullpath);
    critical_exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void sofa_max_create(t_sofa_max* x, t_symbol* s, long argc, t_atom* argv) {
    
    return; // feature disabled
    
    /*outFile.addDim("C", 3);
    outFile.addDim("I", 1);
    outFile.addDim("M", s->M);
    outFile.addDim("R", s->R);
    outFile.addDim("E", s->E);
    outFile.addDim("N", s->N);*/
    
    t_sofaConvention convention;
    long dimensionSize[4] = {0, 0, 0, 0};
    char dimensionName[4] = {'M', 'R', 'E', 'N'};
    
    if(argc < 5) {
        object_error((t_object*)x, "%s: not enough arguments. Expected 5", s->s_name);
        return;
    }
    
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: convention argument must be an integer", s->s_name);
        return;
    }
    convention = (t_sofaConvention)atom_getlong(argv);
    argv++;
    
    for(long i = 0; i < 4; ++i) {
        if(!atomIsANumber(argv)) {
            object_error((t_object*)x, "%s: dimension %s argument must be an integer", s->s_name, dimensionName[i]);
            return;
        }
        dimensionSize[i] = atom_getlong(argv);
        argv++;
    }
    
    if(*x->fileLoaded) {
        csofa_destroySofa(x->sofa);
    }
    
    x->sofa->C = 3;
    x->sofa->I = 1;
    x->sofa->M = dimensionSize[0];
    x->sofa->R = dimensionSize[1];
    x->sofa->E = dimensionSize[2];
    x->sofa->N = dimensionSize[3];
    
    *x->fileLoaded = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void sofa_max_getPositions(t_sofa_max* x, t_symbol* s) {
    if(sofa_max_isFileLoaded(x, gensym("getpositions"))) {
        sofa_getPositions(x, x->outlet_dump, s);
    }
}

void sofa_max_getDimension(t_sofa_max* x, t_symbol* s, long argc, t_atom *argv) {
    long kNumArgs = 1;
    if(argc < kNumArgs) {
        object_error((t_object*)x, "%s: not enough arguments. Expected %d",
                     s->s_name, kNumArgs);
        return;
    }

    if(!sofa_max_isFileLoaded(x, s)) {
        return;
    }
    
    long dim = 0;
    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: argument not a number", s->s_name);
        return;
    }
    dim = atom_getlong(argv);
    if(dim < 0 || dim > SOFA_NUM_DIMENSIONS - 1) {
        return;
    }
    
    if(argc > kNumArgs) {
        object_warn((t_object*)x, "extra arguments for message \"%s\"", s->s_name);
    }
    
    long size = 0;
    switch(dim) {
        case SOFA_M_DIMENSION: size = x->sofa->M; break;
        case SOFA_R_DIMENSION: size = x->sofa->R; break;
        case SOFA_E_DIMENSION: size = x->sofa->E; break;
        case SOFA_N_DIMENSION: size = x->sofa->N; break;
        case SOFA_S_DIMENSION: size = x->sofa->S; break;
    }
    outlet_int(x->outlet_dump, size);
}

void sofa_max_getSize(t_sofa_max* x) {
    if(sofa_max_isFileLoaded(x, gensym("getsizesamps"))) {
        t_atom argv;
        atom_setlong(&argv, x->sofa->N);
        outlet_anything(x->outlet_dump, gensym("sizesamps"), 1, &argv);
    }
}

void sofa_max_getName(t_sofa_max* x) {
    t_atom argv;
    atom_setsym(&argv, x->name);
    outlet_anything(x->outlet_dump, gensym("name"), 1, &argv);
}

void sofa_max_get(t_sofa_max* x, t_symbol* s, long argc, t_atom *argv) {

    // Check arguments
    if(argc < 1) {
        object_error((t_object*)x, "%s: not enough arguments, expecting at least 1", s->s_name);
        return;
    }

    if(!isSofaFileOpen((t_object*)x, x, s)) {
        return;
    }

    if(!atomIsANumber(argv)) {
        object_error((t_object*)x, "%s: data block should be a number", s->s_name);
        return;
    }

    long dataBlock = atom_getlong(argv);
    if(dataBlock < 0 || dataBlock > (x->sofa->numBlocks - 1)) {
        return;
    }
    argv++;

    t_symbol* optionalBufferName = NULL;
    bool optionalBufferIsGiven = false;
    if(argc > 1) {
        if(atom_gettype(argv) != A_SYM) {
            object_error((t_object*)x, "%s: destination buffer name should be a symbol");
            return;
        }
        optionalBufferIsGiven = true;
        optionalBufferName = atom_getsym(argv);
        argv++;
    }

    long buffChanOffset = 0;
    bool optionalBuffChanGiven = false;
    if(argc > 2) {
        if(!atomIsANumber(argv)) {
            object_error((t_object*)x, "%s: buffer channel should be a number", s->s_name);
            return;
        }
        optionalBuffChanGiven = true;
        buffChanOffset = atom_getlong(argv);
        argv++;
    }

    if(argc > 3) {
        object_warn((t_object*)x, "extra arguments for message \"%s\"", s->s_name);
    }

    // Buffer Operations

    t_buffer_ref* buffRef = x->buffRef;
    t_buffer_obj* buffObj;

    if(optionalBufferIsGiven) {
        buffRef = buffer_ref_new((t_object*)x, optionalBufferName);
        if(buffer_ref_exists(buffRef) == 0) {
            object_error((t_object*)x, "%s: destination buffer \"%s\" does not exist", s->s_name,
                         optionalBufferName->s_name);
            object_free(buffRef);
            return;
        }
    }
    else if(buffRef == NULL) {
        object_error((t_object*)x, "%s: no destination buffer has been set", s->s_name);
        return;
    }

    buffObj = buffer_ref_getobject(buffRef);

    long numBuffChans = buffer_getchannelcount(buffObj);
    long numBuffFrames = buffer_getframecount(buffObj);

    if(optionalBuffChanGiven) {
        if(buffChanOffset > (numBuffChans - 1) || buffChanOffset < 0 ) {
            if(optionalBufferIsGiven && buffRef) {
                object_free(buffRef);
            }
            return;
        }
    }

    if(numBuffFrames < x->sofa->N) {
        object_error((t_object*)x, "%s: destination buffer is too short", s->s_name);
        if(optionalBufferIsGiven && buffRef) {
            object_free(buffRef);
        }
        return;
    }

    // Do extraction
    double* d = csofa_getDataIR(x->sofa, dataBlock);
    if(d == NULL) {
        object_error((t_object*)x, "%s: data extraction was unsuccesful", s->s_name);
        if(optionalBufferIsGiven && buffRef) {
            object_free(buffRef);
        }
        return;
    }

    float* buffData = buffer_locksamples(buffObj);
    long buffIndex = 0;
    long N = x->sofa->N;
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
}

bool sofa_max_isFileLoaded(t_sofa_max* x, t_symbol* s) {
    if(!*x->fileLoaded) {
        object_error((t_object*) x, "%s: no SOFA file open", s->s_name);
        return false;
    }
    return true;
}

void sofa_max_assist(t_sofa_max *x, void *b, long m, long a, char *s) {
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
                sprintf(s, "Dump out");
                break;
            case 1:
                sprintf(s, "(bang) File read complete");
                break;
        }
	}
}

void sofa_max_free(t_sofa_max *x) {
    if(x->count) {
        if(*x->count) {
            *x->count -= 1;
        }
        if(*x->count < 1) {
             if(*x->fileLoaded) {
                 csofa_destroySofa(x->sofa);
                 *x->fileLoaded = false;
             }
            
            sysmem_freeptr(x->sofa);
            sysmem_freeptr(x->fileLoaded);
            sysmem_freeptr(x->count);
            globalsymbol_unbind((t_object*)x, x->name->s_name, 0);
            object_unregister(x);
        }
    }
}

void *sofa_max_new(t_symbol *s, long argc, t_atom *argv) {
    t_sofa_max *x = NULL;
    t_symbol* a;

    if((x = (t_sofa_max *)object_alloc((t_class*)sofa_max_class))) {
        a = symbol_unique();
        
        x->sofa = (t_sofa*)sysmem_newptr(sizeof(t_sofa));
        x->fileLoaded = (bool*)sysmem_newptr(sizeof(bool));
        x->count = (long*)sysmem_newptr(sizeof(long));
        *x->fileLoaded = false;
        *x->count = 1;

        if(argc) {
            if(argv->a_type == A_SYM) {
                a = atom_getsym(argv);
                t_sofa_max* ref = (t_sofa_max*)globalsymbol_reference((t_object*)x, a->s_name, "sofa~");
                if(ref != NULL) {
                    /*sysmem_freeptr(x->sofa);
                    sysmem_freeptr(x->fileLoaded);
                    sysmem_freeptr(x->count);

                    x->sofa = ref->sofa;
                    x->fileLoaded = ref->fileLoaded;

                    x->count = ref->count;
                    if(x->count) {
                         *x->count += 1;
                    }*/
                    sysmem_freeptr(x->sofa);
                    sysmem_freeptr(x->fileLoaded);
                    sysmem_freeptr(x->count);
                    object_error((t_object*)x, "Only 1 sofa~ named %s can currently exist", a->s_name);
                    return NULL;
                }
                else {
                    //a->s_thing = (t_object*)x;
                    
                    object_register(APL_SOFA_NAMESPACE, a, x);
                    globalsymbol_bind((t_object*)x, a->s_name, 0);
                }
            }
        }
        else {
            //a->s_thing = (t_object*)x;
            object_register(APL_SOFA_NAMESPACE, a, x);
            globalsymbol_bind((t_object*)x, a->s_name, 0);
        }
        x->name = a;
        x->outlet_finishedLoading = bangout((t_object *)x);
        x->outlet_dump = outlet_new((t_object*)x, NULL);
    }
	return (x);
}
