#include <stdio.h>
#include <assert.h>
#include "greybox.h"
#include "runtime.h"
#include "dm/dm.h"

// this part should be encapsulated in greybox.h
#include "dynamic-array.h"
#include "lts-type.h"

typedef struct group_context {
    model_t             parent;
    int                 len;
    int                *transbegin;
    int                *transmap;
    int                *statemap;
    TransitionCB        cb;
    void               *user_context;
}                  *group_context_t;


static void
group_cb (void *context, int *labels, int *olddst)
{
    group_context_t     ctx = (group_context_t)(context);
    int                 len = ctx->len;
    int                 newdst[len];
    for (int i = 0; i < len; i++)
        newdst[i] = olddst[ctx->statemap[i]];
    ctx->cb (ctx->user_context, labels, newdst);
}

static int
group_long (model_t self, int group, int *newsrc, TransitionCB cb,
            void *user_context)
{
    group_context_t     ctx = (group_context_t)GBgetContext (self);
    model_t             parent = ctx->parent;
    int                 len = ctx->len;
    int                 oldsrc[len];
    int                 Ntrans = 0;
    int                 begin = ctx->transbegin[group];
    int                 end = ctx->transbegin[group + 1];
    ctx->cb = cb;
    ctx->user_context = user_context;

    for (int i = 0; i < len; i++)
        oldsrc[ctx->statemap[i]] = newsrc[i];

    for (int j = begin; j < end; j++) {
        int                 g = ctx->transmap[j];
        Ntrans += GBgetTransitionsLong (parent, g, oldsrc, group_cb, ctx);
    }

    return Ntrans;
}

static int
group_all (model_t self, int *newsrc, TransitionCB cb,
            void *user_context)
{
    group_context_t     ctx = (group_context_t)GBgetContext (self);
    model_t             parent = ctx->parent;
    int                 len = ctx->len;
    int                 oldsrc[len];
    ctx->cb = cb;
    ctx->user_context = user_context;

    for (int i = 0; i < len; i++)
        oldsrc[ctx->statemap[i]] = newsrc[i];

    return GBgetTransitionsAll(parent, oldsrc, group_cb, ctx);
}

int
max_row_first (matrix_t *m, int rowa, int rowb)
{
    int                 i,
                        ra,
                        rb;

    for (i = 0; i < dm_ncols (m); i++) {
        ra = dm_is_set (m, rowa, i);
        rb = dm_is_set (m, rowb, i);

        if ((ra && rb) || (!ra && !rb))
            continue;
        return (rb - ra);
    }

    return 0;
}

int
max_col_first (matrix_t *m, int cola, int colb)
{
    int                 i,
                        ca,
                        cb;

    for (i = 0; i < dm_nrows (m); i++) {
        ca = dm_is_set (m, i, cola);
        cb = dm_is_set (m, i, colb);

        if ((ca && cb) || (!ca && !cb))
            continue;
        return (cb - ca);
    }

    return 0;
}

void GBcopyLTStype(model_t model,lts_type_t info);

static void
apply_regroup_spec (matrix_t *m, const char *spec_)
{
    // parse regrouping arguments
    if (spec_ != NULL) {
        char               *spec = strdup (spec_);
        assert (spec != NULL);

        char               *tok;
        while ((tok = strsep (&spec, ",")) != NULL) {
            if (strcasecmp (tok, "cs") == 0) {
                Warning (info, "Regroup Column Sort");
                dm_sort_cols (m, &max_col_first);
            } else if (strcasecmp (tok, "cn") == 0) {
                Warning (info, "Regroup Column Nub");
                dm_nub_cols (m);
            } else if (strcasecmp (tok, "cw") == 0) {
                Warning (info, "Regroup Column sWaps");
                dm_optimize (m);
            } else if (strcasecmp (tok, "ca") == 0) {
                Warning (info, "Regroup Column All permutations");
                dm_all_perm (m);
            } else if (strcasecmp (tok, "rs") == 0) {
                Warning (info, "Regroup Row Sort");
                dm_sort_rows (m, &max_row_first);
            } else if (strcasecmp (tok, "rn") == 0) {
                Warning (info, "Regroup Row Nub");
                dm_nub_rows (m);
            } else if (strcasecmp (tok, "ru") == 0) {
                Warning (info, "Regroup Row sUbsume");
                dm_subsume_rows (m);
            } else if (strcasecmp (tok, "gs") == 0) {
                const char         *macro = "gc,gr,cw,rs";
                Warning (info, "Regroup macro Group Safely: %s", macro);
                apply_regroup_spec (m, macro);
            } else if (strcasecmp (tok, "ga") == 0) {
                const char         *macro = "gc,rs,ru,cw,rs";
                Warning (info, "Regroup macro Group Aggressively: %s", macro);
                apply_regroup_spec (m, macro);
            } else if (strcasecmp (tok, "gc") == 0) {
                const char         *macro = "cs,cn";
                Warning (info, "Regroup macro Cols: %s", macro);
                apply_regroup_spec (m, macro);
            } else if (strcasecmp (tok, "gr") == 0) {
                const char         *macro = "rs,rn";
                Warning (info, "Regroup macro Rows: %s", macro);
                apply_regroup_spec (m, macro);
            } else if (tok[0] != '\0') {
                Fatal (1, error, "Unknown regrouping specification: '%s'",
                       tok);
            }
        }
        free (spec);
    }
}

model_t
GBregroup (model_t model, const char *regroup_spec)
{
    // note: context information is available via matrix, doesn't need to
    // be stored again
    matrix_t           *m = RTmalloc (sizeof (matrix_t));

    dm_copy (GBgetDMInfo (model), m);

    Warning (info, "Regroup specification: %s", regroup_spec);
    apply_regroup_spec (m, regroup_spec);
    // post processing regroup specification
    // undo column grouping
    dm_ungroup_cols(m);

    // create new model
    model_t             group = GBcreateBase ();
    GBcopyChunkMaps (group, model);
           
    struct group_context *ctx = RTmalloc (sizeof *ctx);

    ctx->parent = model;
    GBsetContext (group, ctx);
    
    GBsetNextStateLong (group, group_long);
    GBsetNextStateAll (group, group_all);

    // fill statemapping: assumption this is a bijection
    {
        int                 Nparts = dm_ncols (m);
        if (Nparts != lts_type_get_state_length (GBgetLTStype (model)))
            Fatal (-1, error,
                   "state mapping in file doesn't match the specification");
        ctx->len = Nparts;
        ctx->statemap = RTmalloc (Nparts * sizeof (int));
        for (int i = 0; i < Nparts; i++) {
            int                 s = m->col_perm.data[i].becomes;
            ctx->statemap[i] = s;
        }
    }

    // fill transition mapping: assumption: this is a surjection
    {
        int                 oldNgroups = dm_nrows (GBgetDMInfo (model));
        int                 newNgroups = dm_nrows (m);
        Warning (info, "Regrouping: %d->%d groups", oldNgroups,
                 newNgroups);
        ctx->transbegin = RTmalloc ((1 + newNgroups) * sizeof (int));
        ctx->transmap = RTmalloc (oldNgroups * sizeof (int));
        int                 p = 0;
        for (int i = 0; i < newNgroups; i++) {
            int                 first = m->row_perm.data[i].becomes;
            int                 all_in_group = first;
            // count
            int                 n = 0;
            do {
                n++;
                all_in_group = m->row_perm.data[all_in_group].group;
            }
            while (all_in_group != first);

            ctx->transbegin[i] = p;
            int                 j = 0;
            do {
                ctx->transmap[p + j] = all_in_group;
                j++;
                all_in_group = m->row_perm.data[all_in_group].group;
            } while (all_in_group != first);
            p = p + n;
        }
        ctx->transbegin[newNgroups] = p;
    }
    
    lts_type_t ltstype=GBgetLTStype (model);
    lts_type_print(debug,ltstype);
    Warning(debug,"permuting ltstype");
    ltstype=lts_type_permute(ltstype,ctx->statemap);
    lts_type_print(debug,ltstype);
    GBsetLTStype (group, ltstype);
 
    // fill edge_info
    GBsetDMInfo (group, m);

    GBinitModelDefaults (&group, model);

    // permute initial state
    {
        int                 len = ctx->len;
        int                 s0[len],
            news0[len];
        GBgetInitialState (model, s0);
        for (int i = 0; i < len; i++)
            news0[i] = s0[ctx->statemap[i]];
        GBsetInitialState (group, news0);
    }

    // not supported yet (should permute states)
    GBsetStateLabelShort (group, NULL);
    GBsetStateLabelLong (group, NULL);
    GBsetStateLabelsAll (group, NULL);

    // who is responsible for freeing matrix_t dm_info in group?
    // probably needed until program termination
    return group;
}