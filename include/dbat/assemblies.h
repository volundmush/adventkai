#pragma once

#include "structs.h"


/* ******************************************************************** *
 * Preprocessor constants.                                             *
 * ******************************************************************** */

/* Assembly type: Used in ASSEMBLY.iAssemblyType */
#define ASSM_MAKE              0       // Assembly must be made.
#define ASSM_BAKE              1       // Assembly must be baked.
#define ASSM_BREW              2       // Assembly must be brewed.
#define ASSM_ASSEMBLE          3       // Assembly must be assembled.
#define ASSM_CRAFT             4       // Assembly must be crafted.
#define ASSM_FLETCH            5       // Assembly must be fletched.
#define ASSM_KNIT              6       // Assembly must be knitted.
#define ASSM_MIX               7       // Assembly must be mixed.
#define ASSM_THATCH            8       // Assembly must be thatched.
#define ASSM_WEAVE             9       // Assembly must be woven.
#define ASSM_FORGE             10      // Assembly must be forged.

/* ******************************************************************** *
 * Type aliases.                                                       *
 * ******************************************************************** */

typedef struct assembly_data ASSEMBLY;
typedef struct component_data COMPONENT;

/* ******************************************************************** *
 * Structure definitions.                                              *
 * ******************************************************************** */

/* Assembly structure definition. */
struct assembly_data {
    long lVnum;                  /* Vnum of the object assembled. */
    long lNumComponents;         /* Number of components. */
    unsigned char uchAssemblyType;        /* Type of assembly (ASSM_xxx).
*/
    struct component_data *pComponents;          /* Array of component info. */
};

/* Assembly component structure definition. */
struct component_data {
    bool bExtract;               /* Extract the object after use. */
    bool bInRoom;                /* Component in room, not inven. */
    long lVnum;                  /* Vnum of the component object. */
};

/* ******************************************************************** *
 * Prototypes for assemblies.c.
*
 * ******************************************************************** */

extern void assemblyBootAssemblies();

extern void assemblySaveAssemblies();

extern void assemblyListToChar(struct char_data *pCharacter);

extern bool assemblyAddComponent(long lVnum, long lComponentVnum,
                                 bool bExtract, bool bInRoom);

extern bool assemblyCheckComponents(long lVnum, struct char_data
*pCharacter, int extract_yes);

extern bool assemblyCreate(long lVnum, int iAssembledType);

extern bool assemblyDestroy(long lVnum);

extern bool assemblyHasComponent(long lVnum, long lComponentVnum);

extern bool assemblyRemoveComponent(long lVnum, long lComponentVnum);

extern int assemblyGetType(long lVnum);

extern long assemblyCountComponents(long lVnum);

extern long assemblyFindAssembly(const char *pszAssemblyName);

extern long assemblyGetAssemblyIndex(long lVnum);

extern long assemblyGetComponentIndex(ASSEMBLY *pAssembly,
                                      long lComponentVnum);

ASSEMBLY *assemblyGetAssemblyPtr(long lVnum);

/* ******************************************************************** */
