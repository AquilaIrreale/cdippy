#include <config.h>

#include <stdlib.h>
#include <stdio.h>

#include "map.h"
#include "adjudicator.h"

#define MAX_ORDERS 34
#define MAX_DEPS 256

struct order orders[MAX_ORDERS];
size_t orders_n;

size_t get_order(enum territory t)
{
    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].terr == t) {
            break;
        }
    }

    return i;
}

void register_order(enum kind kind,
                    enum territory terr,
                    enum territory orig,
                    enum territory targ,
                    enum coast coast,
                    bool via_convoy)
{
    size_t o = get_order(terr);

    if (o == MAX_ORDERS) {
        /* Should normally be unreachable */
        fputs("Too many orders!? Dropping this one\n", stderr);
        return;
    }

    orders[o].kind       = kind;
    orders[o].terr       = terr;
    orders[o].orig       = orig;
    orders[o].targ       = targ;
    orders[o].coast      = coast;
    orders[o].via_convoy = via_convoy;

    if (o == orders_n) {
        orders_n++;
    }
}

enum resolution resolution[MAX_ORDERS];
enum state state[MAX_ORDERS];

size_t deps[MAX_DEPS];
size_t deps_n;

enum resolution resolve(size_t o);
void backup_rule(size_t deps_n_old);
enum resolution adjudicate(size_t o);

void adjudicate_all()
{
    size_t i;
    for (i = 0; i < MAX_ORDERS; i++) {
        state[i] = UNRESOLVED;
    }

    deps_n = 0;

    for (i = 0; i < orders_n; i++) {
        resolve(i);
    }
}

bool convoy_path(enum territory t1, enum territory t2, bool strict);

bool is_legal_move(size_t o, enum territory exclude)
{
    /* Must be a move */
    if (orders[o].kind != MOVE) {
        return false;
    }

    enum territory t1 = orders[o].terr;
    enum territory t2 = orders[o].targ;
    enum coast coast = orders[o].coast;

    if (!territories[t1].occupied) {
        return false;
    }

    if (territories[t1].unit == FLEET) {
        return can_reach(t1, t2, FLEET, coast);
    }

    if (coast != NONE) {
        return false;
    }

    if (can_reach(t1, t2, ARMY, NONE)) {
        return true;
    }

    bool occupied_old;
    if (exclude != NO_TERR) {
        occupied_old = territories[exclude].occupied;
        territories[exclude].occupied = false;
    }

    bool ret = convoy_path(t1, t2, false);

    if (exclude != NO_TERR) {
        territories[exclude].occupied = occupied_old;
    }

    return ret;
}

bool is_legal_convoy(size_t o)
{
    struct order *ord = &orders[o];

    if (ord->kind != CONVOY) {
        return false;
    }

    if (!territories[ord->terr].occupied) {
        return false;
    }

    if (!convoy_path(ord->terr, ord->orig, false) ||
        !convoy_path(ord->terr, ord->targ, false)) {

        return false;
    }

    return true;
}

/* Returns the resolution for order "o"
 */
enum resolution resolve(size_t o)
{
    if (state[o] == RESOLVED) {
        return resolution[o];
    }

    if (state[o] == GUESSING) {
        /* Add "o" to dep list and return the guess */
        deps[deps_n++] = o;
        return resolution[o];
    }

    /* Backup deps_n */
    size_t deps_n_old = deps_n;

    /* Set order state as GUESSING */
    resolution[o] = FAILS;
    state[o] = GUESSING;

    /* Adjudicate order */
    enum resolution res1 = adjudicate(o);

    if (deps_n == deps_n_old) {
        /* Order does not depend on a guess */
        if (state[o] != RESOLVED) {
            resolution[o] = res1;
            state[o] = RESOLVED;
        }

        return res1;
    }

    if (deps[deps_n_old] != o) {
        /* The order depends on a guess that's not our own */
        deps[deps_n++] = o;
        resolution[o] = res1;
        return res1;
    }

    /* Result depends on our guess.
     * Reset the dependencies.
     */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        state[deps[i]] = UNRESOLVED;
    }

    deps_n = deps_n_old;

    /* Try the other guess */
    resolution[o] = SUCCEEDS;
    state[o] = GUESSING;

    enum resolution res2 = adjudicate(o);

    if (res1 == res2) {
        /* The cycle has a unique solution
         * Reset the dependencies and return
         */
        size_t i;
        for (i = deps_n_old; i < deps_n; i++) {
            state[deps[i]] = UNRESOLVED;
        }

        deps_n = deps_n_old;

        resolution[o] = res1;
        state[o] = RESOLVED;
        return res1;
    }

    /* The cycle requires special handling */
    backup_rule(deps_n_old);

    /* Restart from the top to make sure all orders
     * in the cycle have been resolved befor returning
     */
    return resolve(o);
}

bool dislodged(enum territory t)
{
    /* Can't be dislodged if nobody's there */
    if (!territories[t].occupied) {
        return false;
    }

    /* Look for an order for t */
    size_t o = get_order(t);

    if (o < orders_n &&
        orders[o].kind == MOVE &&
        resolve(o) == SUCCEEDS) {

        /* If the unit leaves the territory,
         * it can't be dislodged
         */
        return false;
    }

    /* Look for incoming attacks */
    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].kind == MOVE &&
            orders[i].targ == t &&
            resolve(i) == SUCCEEDS) {

            return true;
        }
    }

    return false;
}

bool convoy_path_r(enum territory t1,
                   enum territory t2,
                   enum territory cur,
                   bool *visited,
                   bool strict,
                   bool legality_check)
{
    /* For each neighbouring sea territory */
    size_t i;
    for (i = 0; i < territories[cur].sea_neighs_n; i++) {
        enum territory t = territories[cur].sea_neighs[i];

        /* If t is our target and we are not on the starting
         * territory, return with success; ignore when checking
         * the legality of a convoy (no need to check for false
         * positives)
         */
        if (t == t2 && (cur != t1 || !legality_check)) {
            return true;
        }

        /* Coastal territories can't convoy, skip */
        if (is_coast(t)) {
            continue;
        }

        /* Skip if we've already been here */
        if (visited[t]) {
            continue;
        }

        /* Convoy cannot be possible if there's no unit in t */
        if (!territories[t].occupied) {
            continue;
        }

        visited[t] = true;

        if (strict) {
            /* Look for an order for t */
            size_t o = get_order(t);

            /* If no order was found, or order is not
             * a convoy, or it is a convoy for a different
             * move than ours, or order is not successful,
             * move on
             */
            if (o == orders_n ||
                orders[o].kind != CONVOY ||
                orders[o].orig != t1 ||
                orders[o].targ != t2 ||
                resolve(o) == FAILS) {

                continue;
            }
        }

        /* A suitable convoy was found in t, explore recursively */
        if (convoy_path_r(t1, t2, t, visited, strict, legality_check)) {
            return true;
        }
    }

    /* No path was found in this branch of the exploration */
    return false;
}

bool convoy_path(enum territory t1, enum territory t2, bool strict)
{
    bool coast_to_coast = is_coast(t1) && is_coast(t2);

    if (strict && !coast_to_coast) {
        return false;
    }

    bool *visited = calloc(TERR_N, sizeof *visited);

    bool ret = convoy_path_r(t1, t2, t1, visited, strict, !coast_to_coast);

    free(visited);
    return ret;
}

bool convoy_intent(size_t o)
{
    if (orders[o].via_convoy) {
        return true;
    }

    enum nation nation = territories[orders[o].terr].nation;

    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].kind == CONVOY &&
            territories[orders[i].terr].nation == nation &&
            orders[i].orig == orders[o].terr &&
            orders[i].targ == orders[o].targ &&
            is_legal_convoy(i)) {

            return true;
        }
    }

    return false;
}

bool path(enum territory t1, enum territory t2, enum coast coast)
{
    if (!territories[t1].occupied) {
        return false;
    }

    if (territories[t1].unit == FLEET) {
        return can_reach(t1, t2, FLEET, coast);
    }

    if (coast != NONE) {
        return false;
    }

    if (convoy_intent(get_order(t1))) {
        return convoy_path(t1, t2, true);
    }

    if (can_reach(t1, t2, ARMY, NONE)) {
        return true;
    }

    return convoy_path(t1, t2, true);
}

unsigned hold_strength(enum territory t)
{
    if (!territories[t].occupied) {
        /* t is empty */
        return 0;
    }

    /* Look for an order for t */
    size_t o = get_order(t);

    /* If order exists and is valid a move */
    if (o != orders_n && is_legal_move(o, NO_TERR)) {

        /* Strength is 0 if it succeds, (occupier unit goes away)
         * 1 otherwise
         */
        return resolve(o) == SUCCEEDS ? 0 : 1;
    }

    /* Count succesful hold supports */
    size_t i;
    unsigned strength = 1;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].kind == SUPPORT &&
            orders[i].orig == t &&
            orders[i].targ == t &&
            resolve(i) == SUCCEEDS) {

            strength++;
        }
    }

    return strength;
}

unsigned successful_supports(enum territory t1,
                             enum territory t2,
                             unsigned excluded)
{
    unsigned ret = 0;

    size_t i;
    for (i = 0; i < orders_n; i++) {
        enum nation sup_nation = territories[orders[i].terr].nation;
        if (orders[i].kind == SUPPORT &&
            orders[i].orig == t1 &&
            orders[i].targ == t2 &&
            sup_nation & ~excluded &&
            resolve(i) == SUCCEEDS) {

            ret++;
        }
    }

    return ret;
}

unsigned attack_strength_vs(enum territory t1,
                            enum territory t2,
                            enum coast coast,
                            enum nation opponent)
{
    if (territories[t1].unit == ARMY) {
        coast = NONE;
    }

    if (!path(t1, t2, coast)) {
        return 0;
    }

    size_t o = get_order(t2);

    bool successful_move = o < orders_n &&
                           orders[o].kind == MOVE &&
                           resolve(o) == SUCCEEDS;

    if (!territories[t2].occupied || successful_move) {
        return 1 + successful_supports(t1, t2, opponent);
    } else {
        return 1 + successful_supports(t1, t2, opponent | territories[t2].nation);
    }
}

unsigned attack_strength(enum territory t1,
                         enum territory t2,
                         enum coast coast)
{
    if (territories[t1].unit == ARMY) {
        coast = NONE;
    }

    if (!path(t1, t2, coast)) {
        return 0;
    }

    size_t o = get_order(t2);

    bool successful_move = o < orders_n &&
                           orders[o].kind == MOVE &&
                           orders[o].targ != t1 &&
                           resolve(o) == SUCCEEDS;

    if (!territories[t2].occupied || successful_move) {
        return 1 + successful_supports(t1, t2, NO_NATION);
    }

    if (territories[t2].nation == territories[t1].nation) {
        return 0;
    }

    return 1 + successful_supports(t1, t2, territories[t2].nation);
}

unsigned defend_strength(enum territory t1,
                         enum territory t2)
{
    return 1 + successful_supports(t1, t2, NO_NATION);
}

bool head_to_head(enum territory t1, enum territory t2)
{
    size_t o1 = get_order(t1);
    size_t o2 = get_order(t2);

    return o1 < orders_n &&
           o2 < orders_n &&
           orders[o1].kind == MOVE &&
           orders[o2].kind == MOVE &&
           orders[o1].targ == t2 &&
           orders[o2].targ == t1 &&
           (!convoy_intent(o1) || !convoy_path(t1, t2, true)) &&
           (!convoy_intent(o2) || !convoy_path(t2, t1, true));
}

unsigned prevent_strength(enum territory t1,
                          enum territory t2,
                          enum coast coast)
{
    if (!path(t1, t2, coast)) {
        return 0;
    }

    if (head_to_head(t1, t2) &&
        resolve(get_order(t2)) == SUCCEEDS) {

        return 0;
    }

    return 1 + successful_supports(t1, t2, NO_NATION);
}

/* Resolves a circular motion
 */
void circular_motion(size_t deps_n_old)
{
    /* Everybody succeeds */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        size_t d = deps[i];
        resolution[d] = SUCCEEDS;
        state[d] = RESOLVED;
    }
}

/* Resolves a convoy paradox
 */
void szykman(size_t deps_n_old)
{
    /* Convoys fail */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        size_t d = deps[i];
        if (orders[d].kind == CONVOY) {
            resolution[d] = FAILS;
            state[d] = RESOLVED;
        } else {
            state[d] = UNRESOLVED;
        }
    }
}

/* Resolves an ambiguous circular dependency
 */
void backup_rule(size_t deps_n_old)
{
    bool convoy = false;
    bool support = false;

    /* Analyze orders involved */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        switch (orders[deps[i]].kind) {
        case CONVOY:
            convoy = true;
            break;
        case SUPPORT:
            support = true;
            break;
        default:
            break;
        }
    }

    if (convoy && support) {
        /* Convoy paradox: apply Szykman rule */
        szykman(deps_n_old);
    } else {
        /* Circular motion */
        circular_motion(deps_n_old);
    }

    deps_n = deps_n_old;
    return;
}

/* Adjudicates order "o"
 */
enum resolution adjudicate(size_t o)
{
    if (!territories[orders[o].terr].occupied) {
        return FAILS;
    }

    switch (orders[o].kind) {
    case CONVOY:
        /* Check territory kind */
        if (!is_inner_sea(orders[o].terr)) {
            return FAILS;
        }

        /* Check that a matching move order exists */
        size_t m = get_order(orders[o].orig);

        if (m >= orders_n ||
            orders[m].kind != MOVE ||
            orders[m].targ != orders[o].targ) {

            return FAILS;
        }

        /* Convoy succeeds if not dislodged */
        if (dislodged(orders[o].terr)) {
            return FAILS;
        }

        return SUCCEEDS;

    case SUPPORT:
        /* Check if valid */
        if (!can_support(orders[o].terr,
                         orders[o].targ,
                         territories[orders[o].terr].unit,
                         territories[orders[o].terr].coast)) {

            return FAILS;
        }

        if (!territories[orders[o].orig].occupied) {
            /* There's no unit to support */
            return FAILS;
        }

        size_t i = get_order(orders[o].orig);

        if (orders[o].orig == orders[o].targ) {
            /* This is a support to hold */
            if (i < orders_n && is_legal_move(i, NO_TERR)) {
                /* Orig unit is ordered a legal move.
                 * An illegal move or no order at all
                 * is treated as a hold. A legal move
                 * cannot be supported to hold, even
                 * when it fails
                 */
                return FAILS;
            }
        } else {
            /* This is a support to move.
             * The order to support must exist, be a
             * legal move and be concordant with us */

            if (i >= orders_n ||
                !is_legal_move(i, orders[o].terr) ||
                orders[i].targ != orders[o].targ) {

                return FAILS;
            }
        }

        /* Check if cut */
        enum territory cur = orders[o].terr;
        for (i = 0; i < orders_n; i++) {
            /* Support is cut by an incoming move */
            if (orders[i].kind != MOVE ||
                orders[i].targ != cur) {

                continue;
            }

            enum territory attacker = orders[i].terr;

            /* By another nation */
            if (territories[attacker].nation
                == territories[cur].nation) {

                continue;
            }

            /* That's not the subject of our supported move */
            if (orders[o].targ == attacker) {
                continue;
            }

            /* And the moving unit has a successcful path */
            if (!path(attacker, cur, orders[i].coast)) {
                continue;
            }

            return FAILS;
        }

        /* Check if dislodged */
        if (dislodged(cur)) {
            return FAILS;
        }

        return SUCCEEDS;

    case MOVE:
        ; /* Null statement, otherwise the compiler complains */

        enum territory t1 = orders[o].terr;
        enum territory t2 = orders[o].targ;
        enum coast coast = orders[o].coast;

        unsigned atk = attack_strength(t1, t2, coast);

        if (head_to_head(t1, t2)) {
            /* It's a head-to-head, use defend_strength */
            if (atk <= defend_strength(t2, t1)) {
                return FAILS;
            }
        } else {
            /* It's a regular battle, use hold_strength */
            if (atk <= hold_strength(t2)) {
                return FAILS;
            }
        }

        /* Check strength agains every other attacker */
        for (i = 0; i < orders_n; i++) {
            if (orders[i].kind == MOVE &&
                orders[i].targ == t2 &&
                orders[i].terr != t1) {

                atk = attack_strength_vs(t1, t2, coast,
                                         territories[orders[i].terr].nation);

                if (atk <= prevent_strength(orders[i].terr, t2,
                                            orders[i].coast)) {
                    return FAILS;
                }
            }
        }

        return SUCCEEDS;

    default:
        return FAILS; /* Should be unreachable */
    }
}
