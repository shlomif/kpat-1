/*
 * dfs.c - the various possible implementations of the function
 * freecell_solver_check_and_add_state().
 * 
 * DFS stands for Depth-First Search, which is the scan used by 
 * Freecell Solver to solve a board.
 *
 * Written by Shlomi Fish (shlomif@vipe.technion.ac.il), 2000
 *
 * This file is in the public domain (it's uncopyrighted).
 */

#include <stdlib.h>
#include <string.h>

#include "fcs_dm.h"
#include "fcs.h"

#if defined(INDIRECT_STATE_STORAGE)||defined(TREE_STATE_STORAGE)||defined(HASH_STATE_STORAGE)||defined(DB_FILE_STATE_STORAGE)
#include "fcs_isa.h"
#endif

#ifdef HASH_STATE_STORAGE
#include "md5.h"
#endif

#ifdef INDIRECT_STACK_STATES
#include "fcs_hash.h"
#include "md5.h"
#endif

#if FCS_METHOD == FCS_METHOD_SOFT_DFS

void freecell_solver_soft_dfs_add_state(
    freecell_solver_instance_t * instance, 
    int depth,
    fcs_state_with_locations_t * state
    );

#elif FCS_METHOD == FCS_METHOD_HARD_DFS

#ifdef DIRECT_STATE_STORAGE
extern int freecell_solver_solve_for_state(freecell_solver_instance_t * instance, fcs_state_with_locations_t state, int depth, int ignore_osins);
#elif defined(INDIRECT_STATE_STORAGE)||defined(TREE_STATE_STORAGE)||defined(HASH_STATE_STORAGE)||defined(HASH_STATE_STORAGE)||defined(DB_FILE_STATE_STORAGE)
extern int freecell_solver_solve_for_state(freecell_solver_instance_t * instance, fcs_state_with_locations_t * state, int depth, int ignore_osins);
#endif

#endif

/*
    The objective of the fcs_caas_check_and_insert macros is:
    1. To check if new_state is already in the prev_states collection.
    2. If not, to add it and to set check to true.
    3. If so, to set check to false.
  */


#ifdef INTERNAL_HASH_IMPLEMENTATION
#define fcs_caas_check_and_insert()              \
    MD5Init(&(instance->md5_context));      \
    MD5Update(&(instance->md5_context), (unsigned char *)new_state, sizeof(fcs_state_t)); \
    MD5Final(instance->hash_value, &(instance->md5_context)); \
    hash_value_int = *(SFO_hash_value_t*)instance->hash_value;      \
    if (hash_value_int < 0)       \
    {    \
        /*             \
         * This is a bit mask that nullifies the sign bit of the  \
         * number so it will always be positive           \
         * */            \
        hash_value_int &= (~(1<<((sizeof(hash_value_int)<<3)-1)));     \
    }    \
    check = (SFO_hash_insert(          \
        instance->hash_value,              \
        new_state,                   \
        hash_value_int               \
        ) == NULL);                  

#elif defined(INDIRECT_STATE_STORAGE)
#define fcs_caas_check_and_insert()              \
    /* Try to see if the state is found in indirect_prev_states */  \
    if (bsearch(&new_state,                                         \
                instance->indirect_prev_states,                     \
                instance->num_indirect_prev_states,                 \
                sizeof(fcs_state_with_locations_t *),               \
                fcs_state_compare_indirect) == NULL)                \
    {                                                               \
        /* It isn't in prev_states, but maybe it's in the sort margin */        \
        pos_ptr = (fcs_state_with_locations_t * *)SFO_bsearch(              \
            &new_state,                                                     \
            instance->indirect_prev_states_margin,                          \
            instance->num_prev_states_margin,                               \
            sizeof(fcs_state_with_locations_t *),                           \
            fcs_state_compare_indirect_with_context,                        \
            NULL,                  \
            &found);              \
                             \
        if (found)                \
        {                             \
            check = 0;                   \
        }                                 \
        else                               \
        {                                     \
            /* Insert the state into its corresponding place in the sort         \
             * margin */                             \
            memmove((void*)(pos_ptr+1), (void*)pos_ptr, sizeof(fcs_state_with_locations_t *)*(instance->num_prev_states_margin-(pos_ptr-instance->indirect_prev_states_margin)));          \
            *pos_ptr = new_state;                \
                       \
            instance->num_prev_states_margin++;             \
              \
            if (instance->num_prev_states_margin >= PREV_STATES_SORT_MARGIN)         \
            {          \
                /* The sort margin is full, let's combine it with the main array */         \
                if (instance->num_indirect_prev_states + instance->num_prev_states_margin > instance->max_num_indirect_prev_states)       \
                {               \
                    while (instance->num_indirect_prev_states + instance->num_prev_states_margin > instance->max_num_indirect_prev_states)        \
                    {            \
                        instance->max_num_indirect_prev_states += PREV_STATES_GROW_BY;         \
                    }                \
                    instance->indirect_prev_states = realloc(instance->indirect_prev_states, sizeof(fcs_state_with_locations_t *) * instance->max_num_indirect_prev_states);       \
                }             \
                            \
                SFO_merge_large_and_small_sorted_arrays(           \
                    instance->indirect_prev_states,              \
                    instance->num_indirect_prev_states,           \
                    instance->indirect_prev_states_margin,          \
                    instance->num_prev_states_margin,              \
                    sizeof(fcs_state_with_locations_t *),           \
                    fcs_state_compare_indirect_with_context,          \
                    NULL                        \
                );                   \
                                  \
                instance->num_indirect_prev_states += instance->num_prev_states_margin;       \
                          \
                instance->num_prev_states_margin=0;           \
            }                  \
            check = 1;               \
        }                  \
                    \
    }                   \
    else                 \
    {         \
        check = 0;          \
    }          

#elif defined(DIRECT_STATE_STORAGE)
#define fcs_caas_check_and_insert()               \
    /* Try to see if the state is found in prev_states */           \
    if (bsearch(new_state,                                         \
                instance->prev_states,          \
                instance->num_prev_states,       \
                sizeof(fcs_state_with_locations_t),       \
                fcs_state_compare) == NULL)            \
                \
    {  \
        /* It isn't in prev_states, but maybe it's in the sort margin */ \
        pos_ptr = (fcs_state_with_locations_t *)SFO_bsearch(        \
            new_state,         \
            instance->prev_states_margin,       \
            instance->num_prev_states_margin,         \
            sizeof(fcs_state_with_locations_t),   \
            fcs_state_compare_with_context,         \
            NULL,            \
            &found);              \
        if (found)            \
        {                \
            check = 0;          \
        }            \
        else           \
        {              \
            /* Insert the state into its corresponding place in the sort      \
             * margin */              \
            memmove((void*)(pos_ptr+1), (void*)pos_ptr, sizeof(fcs_state_with_locations_t) * (instance->num_prev_states_margin - (pos_ptr - instance->prev_states_margin)));         \
            *pos_ptr = *new_state;           \
                 \
            instance->num_prev_states_margin++;         \
           \
            if (instance->num_prev_states_margin >= PREV_STATES_SORT_MARGIN)        \
            {             \
                /* The sort margin is full, let's combine it with the main array */       \
                if (instance->num_prev_states + instance->num_prev_states_margin > instance->max_num_prev_states)      \
                {            \
                    while (instance->num_prev_states + instance->num_prev_states_margin > instance->max_num_prev_states)        \
                    {             \
                        instance->max_num_prev_states += PREV_STATES_GROW_BY;      \
                    }          \
         \
                    instance->prev_states = (fcs_state_with_locations_t *)realloc(instance->prev_states, sizeof(fcs_state_with_locations_t) * instance->max_num_prev_states);        \
                }          \
         \
                SFO_merge_large_and_small_sorted_arrays(      \
                    instance->prev_states,          \
                    instance->num_prev_states,       \
                    instance->prev_states_margin,      \
                    instance->num_prev_states_margin,       \
                    sizeof(fcs_state_with_locations_t), \
                    fcs_state_compare_with_context,        \
                    NULL          \
                );         \
          \
                instance->num_prev_states += instance->num_prev_states_margin;       \
          \
                instance->num_prev_states_margin=0;       \
            }         \
            check = 1;       \
        }         \
          \
    }        \
    else        \
    {   \
        check = 0;        \
    }
#elif defined(LIBREDBLACK_TREE_IMPLEMENTATION)

#define fcs_caas_check_and_insert()               \
    check = (rbsearch(new_state, instance->tree) == new_state);

#elif defined(AVL_AVL_TREE_IMPLEMENTATION)||defined(AVL_REDBLACK_TREE_IMPLEMENTATION)
#ifdef AVL_AVL_TREE_IMPLEMENTATION
#define fcs_libavl_states_tree_insert(a,b) avl_insert((a),(b))
#elif defined(AVL_REDBLACK_TREE_IMPLEMENTATION)
#define fcs_libavl_states_tree_insert(a,b) rb_insert((a),(b))
#endif 

#define fcs_caas_check_and_insert()       \
    check = (fcs_libavl_states_tree_insert(instance->tree, new_state) == NULL);

#elif defined (GLIB_TREE_IMPLEMENTATION)
#define fcs_caas_check_and_insert()       \
    if (g_tree_lookup(instance->tree, (gpointer)new_state) == NULL) \
    {            \
        /* The new state was not found. Let's insert it.       \
         * The value should be non-NULL or else g_hash_table_lookup() will   \
         * return NULL even if it exists. */                  \
        g_tree_insert(                        \
            instance->tree,                      \
            (gpointer)new_state,              \
            (gpointer)0x7348e9f2                 \
            );                         \
        check = 1;                  \
    }              \
    else        \
    {          \
        check = 0;     \
    }


                    
#elif defined (GLIB_HASH_IMPLEMENTATION)
#define fcs_caas_check_and_insert()       \
    if (g_hash_table_lookup(instance->hash, (gpointer)new_state) == NULL) \
    { \
        /* The new state was not found. Let's insert it. \
         * The value should be non-NULL or else g_hash_table_lookup() will \
         * return NULL even if it exists. */       \
        g_hash_table_insert(         \
            instance->hash,          \
            (gpointer)new_state,          \
            (gpointer)0x7348e9f2            \
        \
            );           \
        check = 1;              \
    }          \
    else        \
    {          \
        check = 0;     \
    }

#endif


#ifdef INDIRECT_STACK_STATES
void freecell_solver_cache_stacks(
        freecell_solver_instance_t * instance,
        fcs_state_with_locations_t * new_state
        )
{
    int a;
#if (FCS_STACK_STORAGE == FCS_STACK_STORAGE_INTERNAL_HASH)
    SFO_hash_value_t hash_value_int;
#endif
    void * cached_stack;

    for(a=0 ; a<instance->stacks_num ; a++)
    {
        new_state->s.stacks[a] = realloc(new_state->s.stacks[a], fcs_stack_len(new_state->s, a)+1);
#if FCS_STACK_STORAGE == FCS_STACK_STORAGE_INTERNAL_HASH
        MD5Init(&(instance->md5_context));
        MD5Update(&(instance->md5_context), new_state->s.stacks[a], fcs_stack_len(new_state->s, a)+1);
        MD5Final(instance->hash_value, &(instance->md5_context));
        hash_value_int = *(SFO_hash_value_t*)instance->hash_value;

        if (hash_value_int < 0)
        {
            /*
             * This is a bit mask that nullifies the sign bit of the 
             * number so it will always be positive
             * */
            hash_value_int &= (~(1<<((sizeof(hash_value_int)<<3)-1)));
        }

        cached_stack = (void *)SFO_hash_insert(
            instance->stacks_hash, 
            new_state->s.stacks[a],
            hash_value_int
            );
        
        if (cached_stack != NULL)
        {
            free(new_state->s.stacks[a]);
            new_state->s.stacks[a] = cached_stack;
        }
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBAVL_AVL_TREE) || (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBAVL_REDBLACK_TREE)
        cached_stack = 
#if (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBAVL_AVL_TREE)
            avl_insert(
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBAVL_REDBLACK_TREE)
            rb_insert(    
#endif
            instance->stacks_tree, 
            new_state->s.stacks[a]
            );

        if (cached_stack != NULL)
        {
            free(new_state->s.stacks[a]);
            new_state->s.stacks[a] = cached_stack;
        }

#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_LIBREDBLACK_TREE)
        cached_stack = (void *)rbsearch(
            new_state->s.stacks[a],
            instance->stacks_tree
            );
        if (cached_stack != new_state->s.stacks[a])
        {
            free(new_state->s.stacks[a]);
            new_state->s.stacks[a] = cached_stack;
        }
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GLIB_TREE)
        cached_stack = g_tree_lookup(
             instance->stacks_tree, 
             (gpointer)new_state->s.stacks[a]
             );
        if (cached_stack != NULL)
        {
            free(new_state->s.stacks[a]);
            new_state->s.stacks[a] = cached_stack;
        }
        else
        {
            g_tree_insert(
                instance->stacks_tree,
                (gpointer)new_state->s.stacks[a],
                (gpointer)new_state->s.stacks[a]
                );
        }
#elif (FCS_STACK_STORAGE == FCS_STACK_STORAGE_GLIB_HASH)
        cached_stack = g_hash_table_lookup(
            instance->stacks_hash,
            (gconstpointer)new_state->s.stacks[a]
            );
        if (cached_stack != NULL)
        {
            free(new_state->s.stacks[a]);
            new_state->s.stacks[a] = cached_stack;
        }
        else
        {
            g_hash_table_insert(
                instance->stacks_hash,
                (gpointer)new_state->s.stacks[a],
                (gpointer)new_state->s.stacks[a]
                );
        }
#endif
    }
}
#else
#define freecell_solver_cache_stacks(instance, new_state)
#endif

/*
 * check_and_add_state() does the following things:
 *
 * 1. Check if the number of iterations exceeded its maximum, and if so
 *    return FCS_STATE_EXCEEDS_MAX_NUM_TIMES in order to terminate the
 *    solving process.
 * 2. Check if the maximal depth was reached and if so return 
 *    FCS_STATE_EXCEEDS_MAX_DEPTH
 * 3. Canonize the state.
 * 4. Check if the state is already found in the collection of the states
 *    that were already checked.
 *    If it is:
 *
 *        5a. Return FCS_STATE_ALREADY_EXISTS
 *
 *    If it isn't:
 *    
 *        5b. Call solve_for_state() on the board.
 *
 * */


#if defined(GLIB_HASH_IMPLEMENTATION)
guint freecell_solver_hash_function(gconstpointer key)
{
    MD5_CTX md5_context;
    unsigned char hash_value[MD5_HASHBYTES];
    
    MD5Init(&md5_context);
    MD5Update(&md5_context, (unsigned char *)key, sizeof(fcs_state_t));
    MD5Final(hash_value, &md5_context);

    return (*(guint *)hash_value);
}
#endif

int freecell_solver_check_and_add_state(
    freecell_solver_instance_t * instance, 
    fcs_state_with_locations_t * new_state,
    int depth)
{
#if defined(INTERNAL_HASH_IMPLEMENTATION)
    SFO_hash_value_t hash_value_int;
#endif
#if defined(INDIRECT_STATE_STORAGE)
    fcs_state_with_locations_t * * pos_ptr;
    int found;
#endif
#if defined(DIRECT_STATE_STORAGE)
    fcs_state_with_locations_t * pos_ptr;
    int found;
#endif
#if FCS_METHOD == FCS_METHOD_HARD_DFS
    int ret;
#endif
    int check;

    if ((instance->max_num_times >= 0) &&
        (instance->max_num_times <= instance->num_times))
    {
        return FCS_STATE_BEGIN_SUSPEND_PROCESS;
    }

    if ((instance->max_depth >= 0) &&
        (instance->max_depth <= depth))
    {
        return FCS_STATE_EXCEEDS_MAX_DEPTH;
    }
    
    fcs_canonize_state(new_state, instance->freecells_num, instance->stacks_num);

    freecell_solver_cache_stacks(instance, new_state);

    fcs_caas_check_and_insert();
    if (check)
    {
        /* The new state was not found, and it was already inserted */
#if FCS_METHOD == FCS_METHOD_SOFT_DFS
        freecell_solver_soft_dfs_add_state(instance, depth+1, new_state);
        return FCS_STATE_WAS_SOLVED;
#elif FCS_METHOD == FCS_METHOD_HARD_DFS
        ret = freecell_solver_solve_for_state(instance, 
#if defined DIRECT_STATE_STORAGE
            *new_state
#else
            new_state
#endif
            , depth+1,0);
        if (ret == FCS_STATE_WAS_SOLVED)
        {
            return FCS_STATE_WAS_SOLVED;
        }
        else if (ret == FCS_STATE_SUSPEND_PROCESS)
        {
            return FCS_STATE_SUSPEND_PROCESS;
        }
        else
        {
            return FCS_STATE_IS_NOT_SOLVEABLE;
        }
#endif
    }
    else
    {
        return FCS_STATE_ALREADY_EXISTS;
    }
}



/*
 * This implementation crashes for some reason, so don't use it.
 *
 * */


#ifdef DB_FILE_STATE_STORAGE

static char meaningless_data[16] = "Hello World!";

int freecell_solver_check_and_add_state(freecell_solver_instance_t * instance, fcs_state_with_locations_t * new_state, int depth)
{
    DBT key, value;
    
    if ((instance->max_num_times >= 0) &&
        (instance->max_num_times <= instance->num_times))
    {
        return FCS_STATE_EXCEEDS_MAX_NUM_TIMES;
    }

    if ((instance->max_depth >= 0) &&
        (instance->max_depth <= depth))
    {
        return FCS_STATE_EXCEEDS_MAX_DEPTH;
    }
    
    fcs_canonize_state(new_state, instance->freecells_num, instance->stacks_num);

    freecell_solver_cache_stacks(instance, new_state);

    key.data = new_state;
    key.size = sizeof(*new_state);
    
    if (instance->db->get(
        instance->db,
        NULL,
        &key,
        &value,
        0
        ) == 0)
    {
        /* The new state was not found. Let's insert it. 
         * The value should be non-NULL or else g_hash_table_lookup() will
         * return NULL even if it exists. */

        value.data = meaningless_data;
        value.size = 8;
        instance->db->put(
            instance->db,
            NULL,
            &key,
            &value,
            0);
        if (freecell_solver_solve_for_state(instance, new_state, depth+1,0) == FCS_STATE_WAS_SOLVED)
        {
            return FCS_STATE_WAS_SOLVED;
        }
        else
        {
            return FCS_STATE_IS_NOT_SOLVEABLE;
        }
    }
    else
    {
        /* free (value.data) ; */
        return FCS_STATE_ALREADY_EXISTS;
    }
}


#endif
