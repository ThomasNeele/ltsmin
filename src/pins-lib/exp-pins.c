#include <hre/user.h>
#include <pins-lib/exp-pins.h>
#include <ltsmin-lib/exp-syntax.h>
#include <hre-io/stream.h>
#include <ltsmin-lib/lts-type.h>


static void exp_popt(poptContext con,
                     enum poptCallbackReason reason,
                     const struct poptOption *opt,
                     const char *arg, void *data){
    (void)con;(void)opt;(void)arg;(void)data;
    switch(reason){
        case POPT_CALLBACK_REASON_PRE:
            break;
        case POPT_CALLBACK_REASON_POST:
            GBregisterLoader("exp", EXPloadGreyboxModel);
            Warning(info,"EXP language module initialized");
            return;
        case POPT_CALLBACK_REASON_OPTION:
            break;
    }
    Abort("unexpected call to exp_popt");
}
struct poptOption exp_options[]= {
        {NULL, 0 , POPT_ARG_CALLBACK|POPT_CBFLAG_POST|POPT_CBFLAG_SKIPOPTION , exp_popt, 0 , NULL , NULL },
        POPT_TABLEEND
};

typedef struct greybox_context {
    unsigned int nr_procs;              // Number of processes
    unsigned int bits_act;              // Number of bits for transition label
    unsigned int proc_nrstates;         // Min. nr. of proc. states that fit in 32-bit integer
    unsigned int bits_statevector;      // Number of bits needed for a state vector
    unsigned int *firstbit_statevector; // Offset of the first bit in the state vector for each process
    unsigned int sv_nints;              // Number of integers required for a state vector
    unsigned int *bits_state;           // Number of bits required for each process in the state vector
    unsigned int nbits_offset;          // Size of an offset in the proc_offsets array
    unsigned int max_buf_ints;          // Maximum branching factor
    unsigned int *proc_offsets_start;   // Start of list of offsets in proc_offsets per process
    unsigned int *proc_offsets;         // Offset of each state in proc_trans
    unsigned int nr_trans;              // Total number of transitions in the network
    unsigned int *proc_trans;           // Transitions
    unsigned int nbits_syncbits_offset; // Size of an offset in syncrules
    unsigned int nr_syncbits_offsets;   // Length of the syncbits_offsets array
    unsigned int *syncbits_offsets;     // Offset for each sync rule
    unsigned int nr_syncbits;           // Amount of sync rules
    unsigned int *syncbits;             // Sync rules
} *gb_context_t;

void EXPloadGreyboxModel(model_t model, const char *filename) {

    FILE *fp;
    char stmp[32];

    FILE *in=fopen( filename, "r" );
    if (in == NULL) {
        AbortCall ("Unable to open file ``%s''", filename);
    }
    exp_parse_env_t env = EXPParseEnvCreate();
//    LTSminKeyword(env,TOKEN_END,"end");
//    LTSminKeyword(env,TOKEN_PAR,"par");
//    LTSminKeyword(env,TOKEN_IN,"in");
//    LTSminBinaryOperator(env,TOKEN_STAR,"*",4);
//    LTSminBinaryOperator(env,TOKEN_ARROW,"->",6);
//    LTSminKeyword(env,TOKEN_UNDERSCORE,"_");
//    LTSminBinaryOperator(env,TOKEN_BARS,"||", 5);
//    LTSminKeyword(env,TOKEN_DES,"des");
//    LTSminKeyword(env,TOKEN_LPAR,"(");
//    LTSminKeyword(env,TOKEN_RPAR,")");
//    LTSminKeyword(env,TOKEN_COMMA,",");
//    LTSminKeyword(env,TOKEN_QUOTE,"\"");
//    exp_parse_stream(TOKEN_PAR, env, stream_input(in));

    gb_context_t ctx=(gb_context_t)RTmalloc(sizeof(struct greybox_context));
    GBsetContext(model,ctx);
    lts_type_t ltstype = lts_type_create();
    lts_type_set_edge_label_count(ltstype, 1);
    lts_type_set_state_length(ltstype, ctx->sv_nints);

    GBsetLTStype(model,ltstype);

    // All processes start in state 0
    int* init_state = RTmalloc(sizeof(int)*ctx->sv_nints);
    memset(init_state, 0, sizeof(int)*ctx->sv_nints);
    GBsetInitialState(model, init_state);
}
