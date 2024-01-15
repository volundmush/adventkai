/* ******************************************************************** *
 * FILE        : assemblies.c                  Copyright (C) 1999 Geoff Davis
*
 * USAGE: Implementation for assembly engine.                          *
 * -------------------------------------------------------------------- *
 * 1999 MAY 07 gdavis/azrael@laker.net Initial implementation.         *
 * ******************************************************************** */
#define __ASSEMBLIES_C__

#include "dbat/assemblies.h"
#include "dbat/utils.h"
#include "dbat/comm.h"
#include "dbat/handler.h"
#include "dbat/constants.h"

/* Local global variables. */
static long g_lNumAssemblies = 0;
static ASSEMBLY *g_pAssemblyTable = nullptr;


void assemblyBootAssemblies() {
    char szLine[MAX_STRING_LENGTH] = {'\0'};
    char szTag[MAX_STRING_LENGTH] = {'\0'};
    char szType[MAX_STRING_LENGTH] = {'\0'};
    int iExtract = 0;
    int iInRoom = 0;
    int iType = 0;
    long lLineCount = 0;
    long lPartVnum = NOTHING;
    long lVnum = NOTHING;
    FILE *pFile = nullptr;

    if ((pFile = fopen(ASSEMBLIES_FILE, "rt")) == nullptr) {
        basic_mud_log("SYSERR: assemblyBootAssemblies(): Couldn't open file '%s' for "
            "reading.", ASSEMBLIES_FILE);
        return;
    }

    while (!feof(pFile)) {
        lLineCount += get_line(pFile, szLine);
        half_chop(szLine, szTag, szLine);

        if (*szTag == '\0')
            continue;

        if (strcasecmp(szTag, "Component") == 0) {
            if (sscanf(szLine, "#%ld %d %d", &lPartVnum, &iExtract, &iInRoom) != 3
                    ) {
                basic_mud_log("SYSERR: bootAssemblies(): Invalid format in file %s, line %ld: "
                    "szTag=%s, szLine=%s.", ASSEMBLIES_FILE, lLineCount, szTag, szLine);
            } else if (!assemblyAddComponent(lVnum, lPartVnum, iExtract, iInRoom)) {
                basic_mud_log("SYSERR: bootAssemblies(): Could not add component #%ld to "
                    "assembly #%ld.", lPartVnum, lVnum);
            }
        } else if (strcasecmp(szTag, "Vnum") == 0) {
            if (sscanf(szLine, "#%ld %s", &lVnum, szType) != 2) {
                basic_mud_log("SYSERR: bootAssemblies(): Invalid format in file %s, "
                    "line %ld.", ASSEMBLIES_FILE, lLineCount);
                lVnum = NOTHING;
            } else if ((iType = search_block(szType, AssemblyTypes, true)) < 0) {
                basic_mud_log("SYSERR: bootAssemblies(): Invalid type '%s' for assembly "
                    "vnum #%ld at line %ld.", szType, lVnum, lLineCount);
                lVnum = NOTHING;
            } else if (!assemblyCreate(lVnum, iType)) {
                basic_mud_log("SYSERR: bootAssemblies(): Could not create assembly for vnum "
                    "#%ld, type %s.", lVnum, szType);
                lVnum = NOTHING;
            }
        } else {
            basic_mud_log("SYSERR: Invalid tag '%s' in file %s, line #%ld.", szTag,
                ASSEMBLIES_FILE, lLineCount);
        }

        *szLine = '\0';
        *szTag = '\0';
    }

    fclose(pFile);
}

void assemblySaveAssemblies() {
    char szType[MAX_STRING_LENGTH] = {'\0'};
    long i = 0;
    long j = 0;
    ASSEMBLY *pAssembly = nullptr;
    FILE *pFile = nullptr;

    if ((pFile = fopen(ASSEMBLIES_FILE, "wt")) == nullptr) {
        basic_mud_log("SYSERR: assemblySaveAssemblies(): Couldn't open file '%s' for "
            "writing.", ASSEMBLIES_FILE);
        return;
    }

    for (i = 0; i < g_lNumAssemblies; i++) {
        pAssembly = (g_pAssemblyTable + i);
        sprinttype(pAssembly->uchAssemblyType, AssemblyTypes, szType, sizeof(szType));
        fprintf(pFile, "Vnum                #%ld %s\n", pAssembly->lVnum, szType);

        for (j = 0; j < pAssembly->lNumComponents; j++) {
            fprintf(pFile, "Component           #%ld %d %d\n",
                    pAssembly->pComponents[j].lVnum,
                    (pAssembly->pComponents[j].bExtract ? 1 : 0),
                    (pAssembly->pComponents[j].bInRoom ? 1 : 0));
        }

        if (i < g_lNumAssemblies - 1)
            fprintf(pFile, "\n");
    }

    fclose(pFile);
}

void assemblyListToChar(struct char_data *pCharacter) {
    char szBuffer[MAX_STRING_LENGTH] = {'\0'};
    char szAssmType[MAX_INPUT_LENGTH] = {'\0'};
    long i = 0;                  // Outer iterator.
    long j = 0;                  // Inner iterator.
    long lRnum = 0;              // Object rnum for obj_proto indexing.

    if (pCharacter == nullptr) {
        basic_mud_log("SYSERR: assemblyListAssembliesToChar(): nullptr 'pCharacter'.");
        return;
    } else if (g_pAssemblyTable == nullptr) {
        send_to_char(pCharacter, "No assemblies exist.\r\n");
        return;
    }

    /* Send out a "header" of sorts. */
    send_to_char(pCharacter, "The following assemblies exists:\r\n");

    for (i = 0; i < g_lNumAssemblies; i++) {
        if ((lRnum = real_object(g_pAssemblyTable[i].lVnum)) < 0) {
            send_to_char(pCharacter, "[-----] ***RESERVED***\r\n");
            basic_mud_log("SYSERR: assemblyListToChar(): Invalid vnum #%ld in assembly table.", g_pAssemblyTable[i].lVnum);
        } else {
            sprinttype(g_pAssemblyTable[i].uchAssemblyType, AssemblyTypes, szAssmType, sizeof(szAssmType));
            sprintf(szBuffer, "[%5ld] %s (%s)\r\n", g_pAssemblyTable[i].lVnum,
                    obj_proto[lRnum].short_description, szAssmType);
            send_to_char(pCharacter, szBuffer);

            for (j = 0; j < g_pAssemblyTable[i].lNumComponents; j++) {
                if ((lRnum = real_object(g_pAssemblyTable[i].pComponents[j].lVnum)) < 0) {
                    send_to_char(pCharacter, " -----: ***RESERVED***\r\n");
                    basic_mud_log("SYSERR: assemblyListToChar(): Invalid component vnum #%ld in assembly for vnum #%ld.",
                        g_pAssemblyTable[i].pComponents[j].lVnum, g_pAssemblyTable[i].lVnum);
                } else {
                    sprintf(szBuffer, " %5ld: %-20.20s Extract=%-3.3s InRoom=%-3.3s\r\n",
                            +g_pAssemblyTable[i].pComponents[j].lVnum,
                            obj_proto[lRnum].short_description,
                            (g_pAssemblyTable[i].pComponents[j].bExtract ? "Yes" : "No"),
                            (g_pAssemblyTable[i].pComponents[j].bInRoom ? "Yes" : "No"));
                    send_to_char(pCharacter, szBuffer);
                }
            }
        }
    }
}

bool assemblyAddComponent(long lVnum, long lComponentVnum, bool bExtract, bool bInRoom) {
    ASSEMBLY *pAssembly = nullptr;

    if ((pAssembly = assemblyGetAssemblyPtr(lVnum)) == nullptr) {
        basic_mud_log("SYSERR: assemblyAddComponent(): Invalid 'lVnum' #%ld.", lVnum);
        return (false);
    }
#if CIRCLE_UNSIGNED_INDEX
    else if (real_object(lComponentVnum) == NOTHING)
#else
        else if( real_object( lComponentVnum ) <= NOTHING )
#endif
    {
        basic_mud_log("SYSERR: assemblyAddComponent(): Invalid 'lComponentVnum' #%ld.",
            lComponentVnum);
        return (false);
    }
    /* Removed as of 1.02.29 release */
    /* else if( assemblyHasComponent( lVnum, lComponentVnum ) )
    {
      log( "SYSERR: assemblyAddComponent(): Assembly for vnum #%ld already "
        "has component vnum #%ld.", lVnum, lComponentVnum );
      return (FALSE);
    } */

    /* Create a new component table with room for one more entry. */
    if (pAssembly->pComponents == nullptr)
        CREATE(pAssembly->pComponents, COMPONENT, pAssembly->lNumComponents + 1);
    else
        RECREATE(pAssembly->pComponents, COMPONENT, pAssembly->lNumComponents + 1);

    /*
     * Assign the new component table and setup the new component entry. Then
     * add increment the number of components.
     */

    pAssembly->pComponents[pAssembly->lNumComponents].lVnum = lComponentVnum;
    pAssembly->pComponents[pAssembly->lNumComponents].bExtract = bExtract;
    pAssembly->pComponents[pAssembly->lNumComponents].bInRoom = bInRoom;
    pAssembly->lNumComponents += 1;

    return (true);
}

bool assemblyCheckComponents(long lVnum, struct char_data *pCharacter, int extract_yes) {
    bool bOk = true;
    long i = 0;
    long lRnum = 0;
    struct obj_data **ppComponentObjects = nullptr;
    ASSEMBLY *pAssembly = nullptr;

    if (pCharacter == nullptr) {
        basic_mud_log("SYSERR: nullptr assemblyCheckComponents(): 'pCharacter'.");
        return (false);
    } else if ((pAssembly = assemblyGetAssemblyPtr(lVnum)) == nullptr) {
        basic_mud_log("SYSERR: nullptr assemblyCheckComponents(): Invalid 'lVnum' #%ld.", lVnum);
        return (false);
    }

    if (pAssembly->pComponents == nullptr)
        return (false);
    else if (pAssembly->lNumComponents <= 0)
        return (false);

    CREATE(ppComponentObjects, struct obj_data*, pAssembly->lNumComponents);

    for (i = 0; i < pAssembly->lNumComponents && bOk; i++) {
        if ((lRnum = real_object(pAssembly->pComponents[i].lVnum)) < 0)
            bOk = false;
        else {
            if (pAssembly->pComponents[i].bInRoom) {
                if ((ppComponentObjects[i] = get_obj_in_list_num(lRnum,
                                                                 pCharacter->getRoom()->contents)) == nullptr)
                    bOk = false;
                else {
                    obj_from_room(ppComponentObjects[i]);
                }
            } else {
                if ((ppComponentObjects[i] = get_obj_in_list_num(lRnum,
                                                                 pCharacter->contents)) == nullptr)
                    bOk = false;
                else {
                    obj_from_char(ppComponentObjects[i]);
                }
            }
        }
    }

    for (i = 0; i < pAssembly->lNumComponents; i++) {
        if (ppComponentObjects[i] == nullptr)
            continue;

        if (pAssembly->pComponents[i].bExtract && bOk && extract_yes == true)
            extract_obj(ppComponentObjects[i]);
        else if (pAssembly->pComponents[i].bInRoom)
            obj_to_room(ppComponentObjects[i], IN_ROOM(pCharacter));
        else
            obj_to_char(ppComponentObjects[i], pCharacter);
    }

    free(ppComponentObjects);

    return (bOk);
}

bool assemblyCreate(long lVnum, int iAssembledType) {
    long lBottom = 0;
    long lMiddle = 0;
    long lTop = 0;
    ASSEMBLY *pNewAssemblyTable = nullptr;

    if (lVnum < 0)
        return (false);
    else if (iAssembledType < 0 || iAssembledType >= MAX_ASSM)
        return (false);

    if (g_pAssemblyTable == nullptr) {
        CREATE(g_pAssemblyTable, ASSEMBLY, 1);
        g_lNumAssemblies = 1;
    } else {
        lTop = g_lNumAssemblies - 1;

        for (;;) {
            lMiddle = (lBottom + lTop) / 2;

            if (g_pAssemblyTable[lMiddle].lVnum == lVnum)
                return (false);
            else if (lBottom >= lTop)
                break;
            else if (g_pAssemblyTable[lMiddle].lVnum > lVnum)
                lTop = lMiddle - 1;
            else
                lBottom = lMiddle + 1;
        }

        if (g_pAssemblyTable[lMiddle].lVnum <= lVnum)
            lMiddle += 1;

        CREATE(pNewAssemblyTable, ASSEMBLY, g_lNumAssemblies + 1);

        if (lMiddle > 0)
            memmove(pNewAssemblyTable, g_pAssemblyTable, lMiddle * sizeof(ASSEMBLY
            ));

        if (lMiddle <= g_lNumAssemblies - 1)
            memmove(pNewAssemblyTable + lMiddle + 1, g_pAssemblyTable + lMiddle,
                    (g_lNumAssemblies - lMiddle) * sizeof(ASSEMBLY));

        free(g_pAssemblyTable);
        g_pAssemblyTable = pNewAssemblyTable;
        g_lNumAssemblies += 1;
    }

    g_pAssemblyTable[lMiddle].lNumComponents = 0;
    g_pAssemblyTable[lMiddle].lVnum = lVnum;
    g_pAssemblyTable[lMiddle].pComponents = nullptr;
    g_pAssemblyTable[lMiddle].uchAssemblyType = (unsigned char) iAssembledType;
    return (true);
}

bool assemblyDestroy(long lVnum) {
    long lIndex = 0;
    ASSEMBLY *pNewAssemblyTable = nullptr;

    /* Find the real number of the assembled vnum. */
    if (g_pAssemblyTable == nullptr || (lIndex = assemblyGetAssemblyIndex(lVnum))
                                       < 0) {
        basic_mud_log("SYSERR: assemblyDestroy(): Invalid 'lVnum' #%ld.", lVnum);
        return (false);
    }

    /* Deallocate component array. */
    if (g_pAssemblyTable[lIndex].pComponents != nullptr)
        free(g_pAssemblyTable[lIndex].pComponents);

    if (g_lNumAssemblies > 1) {
        /* Create a new table, the same size as the old one less one item. */
        CREATE(pNewAssemblyTable, ASSEMBLY, g_lNumAssemblies - 1);

        /* Copy all assemblies before the one removed into the new table. */
        if (lIndex > 0)
            memmove(pNewAssemblyTable, g_pAssemblyTable, lIndex * sizeof(ASSEMBLY));

        /* Copy all assemblies after the one removed into the new table. */
        if (lIndex < g_lNumAssemblies - 1) {
            memmove(pNewAssemblyTable + lIndex, g_pAssemblyTable + lIndex + 1, (g_lNumAssemblies - lIndex - 1) *
                                                                               sizeof(ASSEMBLY));
        }
    }

    /* Deallocate the old table. */
    free(g_pAssemblyTable);

    /* Decrement the assembly count and assign the new table. */
    g_lNumAssemblies -= 1;
    g_pAssemblyTable = pNewAssemblyTable;

    return (true);
}

bool assemblyHasComponent(long lVnum, long lComponentVnum) {
    ASSEMBLY *pAssembly = nullptr;

    if ((pAssembly = assemblyGetAssemblyPtr(lVnum)) == nullptr) {
        basic_mud_log("SYSERR: assemblyHasComponent(): Invalid 'lVnum' #%ld.", lVnum);
        return (false);
    }

    return (assemblyGetComponentIndex(pAssembly, lComponentVnum) >= 0);
}

bool assemblyRemoveComponent(long lVnum, long lComponentVnum) {
    long lIndex = 0;
    ASSEMBLY *pAssembly = nullptr;
    COMPONENT *pNewComponents = nullptr;

    if ((pAssembly = assemblyGetAssemblyPtr(lVnum)) == nullptr) {
        basic_mud_log("SYSERR: assemblyRemoveComponent(): Invalid 'lVnum' #%ld.", lVnum);
        return (false);
    } else if ((lIndex = assemblyGetComponentIndex(pAssembly, lComponentVnum)) <
               0) {
        basic_mud_log("SYSERR: assemblyRemoveComponent(): Vnum #%ld is not a "
            "component of assembled vnum #%ld.", lComponentVnum, lVnum);
        return (false);
    }

    if (pAssembly->pComponents != nullptr && pAssembly->lNumComponents > 1) {
        CREATE(pNewComponents, COMPONENT, pAssembly->lNumComponents - 1);

        if (lIndex > 0)
            memmove(pNewComponents, pAssembly->pComponents, lIndex * sizeof(COMPONENT));

        if (lIndex < pAssembly->lNumComponents - 1) {
            memmove(pNewComponents + lIndex, pAssembly->pComponents + lIndex + 1,
                    (pAssembly->lNumComponents - lIndex - 1) * sizeof(COMPONENT));
        }
    }

    free(pAssembly->pComponents);
    pAssembly->pComponents = pNewComponents;
    pAssembly->lNumComponents -= 1;

    return (true);
}

int assemblyGetType(long lVnum) {
    ASSEMBLY *pAssembly = nullptr;

    if ((pAssembly = assemblyGetAssemblyPtr(lVnum)) == nullptr) {
        basic_mud_log("SYSERR: assemblyGetType(): Invalid 'lVnum' #%ld.", lVnum);
        return (-1);
    }

    return ((int) pAssembly->uchAssemblyType);
}

long assemblyCountComponents(long lVnum) {
    ASSEMBLY *pAssembly = nullptr;

    if ((pAssembly = assemblyGetAssemblyPtr(lVnum)) == nullptr) {
        basic_mud_log("SYSERR: assemblyCountComponents(): Invalid 'lVnum' #%ld.", lVnum);
        return (0);
    }

    return (pAssembly->lNumComponents);
}

long assemblyFindAssembly(const char *pszAssemblyName) {
    long i = 0;
    long lRnum = NOTHING;

    if (g_pAssemblyTable == nullptr)
        return (-1);
    else if (pszAssemblyName == nullptr || *pszAssemblyName == '\0')
        return (-1);

    for (i = 0; i < g_lNumAssemblies; i++) {
        if ((lRnum = real_object(g_pAssemblyTable[i].lVnum)) < 0)
            basic_mud_log("SYSERR: assemblyFindAssembly(): Invalid vnum #%ld in assembly table.", g_pAssemblyTable[i].lVnum);
        else if (isname(pszAssemblyName, obj_proto[lRnum].name))
            return (g_pAssemblyTable[i].lVnum);
    }

    return (-1);
}

long assemblyGetAssemblyIndex(long lVnum) {
    long lBottom = 0;
    long lMiddle = 0;
    long lTop = 0;

    lTop = g_lNumAssemblies - 1;

    for (;;) {
        lMiddle = (lBottom + lTop) / 2;

        if (g_pAssemblyTable[lMiddle].lVnum == lVnum)
            return (lMiddle);
        else if (lBottom >= lTop)
            return (-1);
        else if (g_pAssemblyTable[lMiddle].lVnum > lVnum)
            lTop = lMiddle - 1;
        else
            lBottom = lMiddle + 1;
    }
}

long assemblyGetComponentIndex(ASSEMBLY *pAssembly, long lComponentVnum) {
    long i = 0;

    if (pAssembly == nullptr)
        return (-1);

    for (i = 0; i < pAssembly->lNumComponents; i++) {
        if (pAssembly->pComponents[i].lVnum == lComponentVnum)
            return (i);
    }

    return (-1);
}

ASSEMBLY *assemblyGetAssemblyPtr(long lVnum) {
    long lIndex = 0;

    if (g_pAssemblyTable == nullptr)
        return (nullptr);

    if ((lIndex = assemblyGetAssemblyIndex(lVnum)) >= 0)
        return (g_pAssemblyTable + lIndex);

    return (nullptr);
}

#undef __ASSEMBLIES_C__

void free_assemblies() {
    int i;

    if (g_pAssemblyTable == nullptr)
        return;

    for (i = 0; i < g_lNumAssemblies; i++) {
        if (g_pAssemblyTable[i].pComponents)
            free(g_pAssemblyTable[i].pComponents);

        g_pAssemblyTable[i].lNumComponents = 0;
    }

    free(g_pAssemblyTable);
}

