/*
 *  cdippy: a classic Diplomacy adjudicator, in C
 *  Copyright (C) 2018 Simone Cimarelli a.k.a. AquilaIrreale
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "map.h"
#include "adjudicator.h"

#define MAX_DEPS 256

struct order orders[MAX_ORDERS];
size_t orders_n;

size_t cd_get_order(enum cd_terr t)
{
    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].terr == t) {
            break;
        }
    }

    return i;
}

void cd_register_order(enum kind kind,
                       enum cd_terr terr,
                       enum cd_terr orig,
                       enum cd_terr targ,
                       enum cd_coast coast,
                       bool via_convoy)
{
    size_t o = cd_get_order(terr);

    assert(o < MAX_ORDERS);

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

void cd_register_move(enum cd_terr t2, enum cd_terr t3, enum cd_coast coast, bool via_convoy)
{
    cd_register_order(MOVE, t2, t2, t3, coast, via_convoy);
}

void cd_register_suph(enum cd_terr t1, enum cd_terr t2)
{
    cd_register_order(SUPPORT, t1, t2, t2, NO_COAST, false);
}

void cd_register_supm(enum cd_terr t1, enum cd_terr t2, enum cd_terr t3)
{
    cd_register_order(SUPPORT, t1, t2, t3, NO_COAST, false);
}

void cd_register_conv(enum cd_terr t1, enum cd_terr t2, enum cd_terr t3)
{
    cd_register_order(CONVOY, t1, t2, t3, NO_COAST, false);
}

enum cd_resolution cd_resolutions[MAX_ORDERS];
enum state states[MAX_ORDERS];

size_t deps[MAX_DEPS];
size_t deps_n;

struct cd_retreat cd_retreats[MAX_ORDERS];
size_t cd_retreats_n;

enum cd_resolution cd_resolve(size_t o);
void cd_backup_rule(size_t deps_n_old);
enum cd_resolution cd_adjudicate(size_t o);

void cd_adjudicate_all()
{
    size_t i;
    for (i = 0; i < MAX_ORDERS; i++) {
        states[i] = UNRESOLVED;
    }

    deps_n = 0;

    for (i = 0; i < orders_n; i++) {
        cd_resolve(i);
    }
}

void cd_compute_retreats()
{
    cd_retreats_n = 0;

    enum cd_terr t1;
    for (t1 = 0; t1 < TERR_N; t1++) {
        if (!cd_dislodged(t1)) {
            continue;
        }

        cd_retreats[cd_retreats_n].who = t1;

        const enum cd_terr *neighs = territories[t1].unit == ARMY
                                     ? territories[t1].land_neighs
                                     : territories[t1].sea_neighs;

        size_t neighs_n = territories[t1].unit == ARMY
                        ? territories[t1].land_neighs_n
                        : territories[t1].sea_neighs_n;

        size_t i, j = 0;
        for (i = 0; i < neighs_n; i++) {
            enum cd_terr t2 = neighs[i];

            if (cd_can_retreat(t1, t2)) {
                cd_retreats[cd_retreats_n].where[j++] = t2;
            }
        }

        cd_retreats[cd_retreats_n].where_n = j;

        cd_retreats_n++;
    }
}

void cd_run_adjudicator()
{
    cd_adjudicate_all();
    cd_compute_retreats();
}

bool cd_convoy_path(enum cd_terr t1,
                    enum cd_terr t2,
                    bool strict,
                    bool check_outcome);

bool cd_is_legal_move(size_t o, enum cd_terr exclude)
{
    /* Must be a move */
    if (orders[o].kind != MOVE) {
        return false;
    }

    enum cd_terr t1 = orders[o].terr;
    enum cd_terr t2 = orders[o].targ;
    enum cd_coast coast = orders[o].coast;

    if (!territories[t1].occupied) {
        return false;
    }

    if (territories[t1].unit == FLEET) {
        return cd_can_reach(t1, t2, FLEET, coast);
    }

    if (coast != NO_COAST) {
        return false;
    }

    if (cd_can_reach(t1, t2, ARMY, NO_COAST)) {
        return true;
    }

    if (!cd_is_land(t2)) {
        return false;
    }

    bool occupied_old;
    if (exclude != NO_TERR) {
        occupied_old = territories[exclude].occupied;
        territories[exclude].occupied = false;
    }

    bool ret = cd_convoy_path(t1, t2, false, false);

    if (exclude != NO_TERR) {
        territories[exclude].occupied = occupied_old;
    }

    return ret;
}

bool cd_is_legal_convoy(size_t o)
{
    struct order *ord = &orders[o];

    if (ord->kind != CONVOY) {
        return false;
    }

    if (!territories[ord->terr].occupied) {
        return false;
    }

    if (!cd_convoy_path(ord->terr, ord->orig, false, false) ||
        !cd_convoy_path(ord->terr, ord->targ, false, false)) {

        return false;
    }

    return true;
}

/* Returns the resolution for order "o"
 */
enum cd_resolution cd_resolve(size_t o)
{
    if (states[o] == RESOLVED) {
        return cd_resolutions[o];
    }

    if (states[o] == GUESSING) {
        /* Add "o" to dep list and return the guess */
        deps[deps_n++] = o;
        return cd_resolutions[o];
    }

    /* Backup deps_n */
    size_t deps_n_old = deps_n;

    /* Set order state as GUESSING */
    cd_resolutions[o] = FAILS;
    states[o] = GUESSING;

    /* Adjudicate order */
    enum cd_resolution res1 = cd_adjudicate(o);

    if (deps_n == deps_n_old) {
        /* Order does not depend on a guess */
        if (states[o] != RESOLVED) {
            cd_resolutions[o] = res1;
            states[o] = RESOLVED;
        }

        return res1;
    }

    if (deps[deps_n_old] != o) {
        /* The order depends on a guess that's not our own */
        deps[deps_n++] = o;
        cd_resolutions[o] = res1;
        return res1;
    }

    /* Result depends on our guess.
     * Reset the dependencies.
     */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        states[deps[i]] = UNRESOLVED;
    }

    deps_n = deps_n_old;

    /* Try the other guess */
    cd_resolutions[o] = SUCCEEDS;
    states[o] = GUESSING;

    enum cd_resolution res2 = cd_adjudicate(o);

    if (res1 == res2) {
        /* The cycle has a unique solution
         * Reset the dependencies and return
         */
        size_t i;
        for (i = deps_n_old; i < deps_n; i++) {
            states[deps[i]] = UNRESOLVED;
        }

        deps_n = deps_n_old;

        cd_resolutions[o] = res1;
        states[o] = RESOLVED;
        return res1;
    }

    /* The cycle requires special handling */
    cd_backup_rule(deps_n_old);

    /* Restart from the top to make sure all orders
     * in the cycle have been resolved befor returning
     */
    return cd_resolve(o);
}

bool cd_dislodged(enum cd_terr t)
{
    /* Can't be dislodged if nobody's there */
    if (!territories[t].occupied) {
        return false;
    }

    /* Look for an order for t */
    size_t o = cd_get_order(t);

    if (o < orders_n &&
        orders[o].kind == MOVE &&
        cd_resolve(o) == SUCCEEDS) {

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
            cd_resolve(i) == SUCCEEDS) {

            return true;
        }
    }

    return false;
}

bool cd_convoy_path_r(enum cd_terr t1,
                      enum cd_terr t2,
                      enum cd_terr cur,
                      bool *visited,
                      bool strict,
                      bool legality_check,
                      bool check_outcome)
{
    /* For each neighbouring sea territory */
    size_t i;
    for (i = 0; i < territories[cur].sea_neighs_n; i++) {
        enum cd_terr t = territories[cur].sea_neighs[i];

        /* If t is our target and we are not on the starting
         * territory, return with success; ignore when checking
         * the legality of a convoy (no need to check for false
         * positives)
         */
        if (t == t2 && (cur != t1 || legality_check)) {
            return true;
        }

        /* Coastal territories can't convoy, skip */
        if (cd_is_coast(t)) {
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
            size_t o = cd_get_order(t);

            /* If no order was found, or order is not
             * a convoy, or it is a convoy for a different
             * move than ours, or order is not successful,
             * move on
             */
            if (o == orders_n ||
                orders[o].kind != CONVOY ||
                orders[o].orig != t1 ||
                orders[o].targ != t2 ||
                (check_outcome && cd_resolve(o) == FAILS)) {

                continue;
            }
        }

        /* A suitable convoy was found in t, explore recursively */
        if (cd_convoy_path_r(t1, t2, t, visited, strict, legality_check, check_outcome)) {
            return true;
        }
    }

    /* No path was found in this branch of the exploration */
    return false;
}

bool cd_convoy_path(enum cd_terr t1,
                    enum cd_terr t2,
                    bool strict,
                    bool check_outcome)
{
    bool coast_to_coast = cd_is_coast(t1) && cd_is_coast(t2);

    if (strict && !coast_to_coast) {
        return false;
    }

    bool *visited = calloc(TERR_N, sizeof *visited);

    bool ret = cd_convoy_path_r(t1, t2, t1, visited,
                                strict, !coast_to_coast, check_outcome);

    free(visited);
    return ret;
}

bool cd_convoy_intent(size_t o)
{
    if (orders[o].via_convoy) {
        return true;
    }

    enum cd_nation nation = territories[orders[o].terr].nation;

    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].kind == CONVOY &&
            territories[orders[i].terr].nation == nation &&
            orders[i].orig == orders[o].terr &&
            orders[i].targ == orders[o].targ &&
            cd_is_legal_convoy(i)) {

            return true;
        }
    }

    return false;
}

bool cd_path(enum cd_terr t1, enum cd_terr t2, enum cd_coast coast)
{
    if (!territories[t1].occupied) {
        return false;
    }

    if (territories[t1].unit == FLEET) {
        return cd_can_reach(t1, t2, FLEET, coast);
    }

    if (coast != NO_COAST) {
        return false;
    }

    bool can_reach_ret = cd_can_reach(t1, t2, ARMY, NO_COAST);

    if (!can_reach_ret || (cd_convoy_path(t1, t2, true, false) &&
                           cd_convoy_intent(cd_get_order(t1)))) {

        return cd_convoy_path(t1, t2, true, true);
    }

    return can_reach_ret;
}

unsigned cd_hold_strength(enum cd_terr t)
{
    if (!territories[t].occupied) {
        /* t is empty */
        return 0;
    }

    /* Look for an order for t */
    size_t o = cd_get_order(t);

    /* If order exists and is valid a move */
    if (o != orders_n && cd_is_legal_move(o, NO_TERR)) {

        /* Strength is 0 if it succeds, (occupier unit goes away)
         * 1 otherwise
         */
        return cd_resolve(o) == SUCCEEDS ? 0 : 1;
    }

    /* Count succesful hold supports */
    size_t i;
    unsigned strength = 1;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].kind == SUPPORT &&
            orders[i].orig == t &&
            orders[i].targ == t &&
            cd_resolve(i) == SUCCEEDS) {

            strength++;
        }
    }

    return strength;
}

unsigned cd_successful_supports(enum cd_terr t1,
                                enum cd_terr t2,
                                unsigned excluded)
{
    unsigned ret = 0;

    size_t i;
    for (i = 0; i < orders_n; i++) {
        enum cd_nation sup_nation = territories[orders[i].terr].nation;
        if (orders[i].kind == SUPPORT &&
            orders[i].orig == t1 &&
            orders[i].targ == t2 &&
            sup_nation & ~excluded &&
            cd_resolve(i) == SUCCEEDS) {

            ret++;
        }
    }

    return ret;
}

unsigned cd_attack_strength_vs(enum cd_terr t1,
                               enum cd_terr t2,
                               enum cd_coast coast,
                               enum cd_nation opponent)
{
    if (territories[t1].unit == ARMY) {
        coast = NO_COAST;
    }

    if (!cd_path(t1, t2, coast)) {
        return 0;
    }

    size_t o = cd_get_order(t2);

    bool successful_move = o < orders_n &&
                           orders[o].kind == MOVE &&
                           cd_resolve(o) == SUCCEEDS;

    if (!territories[t2].occupied || successful_move) {
        return 1 + cd_successful_supports(t1, t2, opponent);
    } else {
        return 1 + cd_successful_supports(t1, t2, opponent | territories[t2].nation);
    }
}

unsigned cd_attack_strength(enum cd_terr t1,
                            enum cd_terr t2,
                            enum cd_coast coast)
{
    if (territories[t1].unit == ARMY) {
        coast = NO_COAST;
    }

    if (!cd_path(t1, t2, coast)) {
        return 0;
    }

    size_t o = cd_get_order(t2);

    bool successful_move = o < orders_n &&
                           orders[o].kind == MOVE &&
                           orders[o].targ != t1 &&
                           cd_resolve(o) == SUCCEEDS;

    if (!territories[t2].occupied || successful_move) {
        return 1 + cd_successful_supports(t1, t2, NO_NATION);
    }

    if (territories[t2].nation == territories[t1].nation) {
        return 0;
    }

    return 1 + cd_successful_supports(t1, t2, territories[t2].nation);
}

unsigned cd_defend_strength(enum cd_terr t1,
                            enum cd_terr t2)
{
    return 1 + cd_successful_supports(t1, t2, NO_NATION);
}

bool cd_head_to_head(enum cd_terr t1, enum cd_terr t2, bool retreat)
{
    size_t o2 = cd_get_order(t2);

    if (o2 >= orders_n ||
        orders[o2].kind != MOVE ||
        orders[o2].targ != t1 ||
        (cd_convoy_intent(o2) && cd_convoy_path(t2, t1, true, true))) {

        return false;
    }

    if (retreat) {
        return true;
    }

    size_t o1 = cd_get_order(t1);

    return o1 < orders_n &&
           orders[o1].kind == MOVE &&
           orders[o1].targ == t2 &&
           (!cd_convoy_intent(o1) || !cd_convoy_path(t1, t2, true, true));
}

unsigned cd_prevent_strength(enum cd_terr t1,
                             enum cd_terr t2,
                             enum cd_coast coast)
{
    if (!cd_path(t1, t2, coast)) {
        return 0;
    }

    if (cd_head_to_head(t1, t2, false) &&
        cd_resolve(cd_get_order(t2)) == SUCCEEDS) {

        return 0;
    }

    return 1 + cd_successful_supports(t1, t2, NO_NATION);
}

bool cd_can_retreat(enum cd_terr t1, enum cd_terr t2)
{
    if (cd_hold_strength(t2) > 0) {
        return false;
    }

    if (cd_head_to_head(t1, t2, true)) {
        return false;
    }

    size_t i;
    for (i = 0; i < orders_n; i++) {
        if (orders[i].kind == MOVE &&
            orders[i].targ == t2 &&
            orders[i].terr != t1) {

            if (cd_prevent_strength(orders[i].terr, t2,
                orders[i].coast) > 0) {

                return false;
            }
        }
    }

    return true;
}

/* Resolves a circular motion
 */
void cd_circular_motion(size_t deps_n_old)
{
    /* Everybody succeeds */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        size_t d = deps[i];
        cd_resolutions[d] = SUCCEEDS;
        states[d] = RESOLVED;
    }
}

/* Resolves a convoy paradox
 */
void cd_szykman(size_t deps_n_old)
{
    /* Convoys fail */
    size_t i;
    for (i = deps_n_old; i < deps_n; i++) {
        size_t d = deps[i];
        if (orders[d].kind == CONVOY) {
            cd_resolutions[d] = FAILS;
            states[d] = RESOLVED;
        } else {
            states[d] = UNRESOLVED;
        }
    }
}

/* Resolves an ambiguous circular dependency
 */
void cd_backup_rule(size_t deps_n_old)
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
        cd_szykman(deps_n_old);
    } else {
        /* Circular motion */
        cd_circular_motion(deps_n_old);
    }

    deps_n = deps_n_old;
    return;
}

/* Adjudicates order "o"
 */
enum cd_resolution cd_adjudicate(size_t o)
{
    if (!territories[orders[o].terr].occupied) {
        return FAILS;
    }

    switch (orders[o].kind) {
    case CONVOY:
        /* Check territory kind */
        if (!cd_is_inner_sea(orders[o].terr)) {
            return FAILS;
        }

        /* Check that a matching move order exists */
        size_t m = cd_get_order(orders[o].orig);

        if (m >= orders_n ||
            orders[m].kind != MOVE ||
            orders[m].targ != orders[o].targ) {

            return FAILS;
        }

        /* Convoy succeeds if not dislodged */
        if (cd_dislodged(orders[o].terr)) {
            return FAILS;
        }

        return SUCCEEDS;

    case SUPPORT:
        /* Check if valid */
        if (!cd_can_support(orders[o].terr,
            orders[o].targ,
            territories[orders[o].terr].unit,
            territories[orders[o].terr].coast)) {

            return FAILS;
        }

        if (!territories[orders[o].orig].occupied) {
            /* There's no unit to support */
            return FAILS;
        }

        size_t i = cd_get_order(orders[o].orig);

        if (orders[o].orig == orders[o].targ) {
            /* This is a support to hold */
            if (i < orders_n && cd_is_legal_move(i, NO_TERR)) {
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
                !cd_is_legal_move(i, orders[o].terr) ||
                orders[i].targ != orders[o].targ) {

                return FAILS;
            }
        }

        /* Check if cut */
        enum cd_terr cur = orders[o].terr;
        for (i = 0; i < orders_n; i++) {
            /* Support is cut by an incoming move */
            if (orders[i].kind != MOVE ||
                orders[i].targ != cur) {

                continue;
            }

            enum cd_terr attacker = orders[i].terr;

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
            if (!cd_path(attacker, cur, orders[i].coast)) {
                continue;
            }

            return FAILS;
        }

        /* Check if dislodged */
        if (cd_dislodged(cur)) {
            return FAILS;
        }

        return SUCCEEDS;

    case MOVE:
        ; /* Null statement, otherwise the compiler complains */

        enum cd_terr t1 = orders[o].terr;
        enum cd_terr t2 = orders[o].targ;
        enum cd_coast coast = orders[o].coast;

        unsigned atk = cd_attack_strength(t1, t2, coast);

        if (cd_head_to_head(t1, t2, false)) {
            /* It's a head-to-head, use defend_strength */
            if (atk <= cd_defend_strength(t2, t1)) {
                return FAILS;
            }
        } else {
            /* It's a regular battle, use hold_strength */
            if (atk <= cd_hold_strength(t2)) {
                return FAILS;
            }
        }

        /* Check strength agains every other attacker */
        for (i = 0; i < orders_n; i++) {
            if (orders[i].kind == MOVE &&
                orders[i].targ == t2 &&
                orders[i].terr != t1) {

                atk = cd_attack_strength_vs(t1, t2, coast,
                                         territories[orders[i].terr].nation);

                if (atk <= cd_prevent_strength(orders[i].terr, t2,
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
