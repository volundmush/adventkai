/************************************************************************
 * Generic OLC Library - Objects / genobj.h			v1.0	*
 * Original author: Levork						*
 * Copyright 1996 by Harvey Gilpin					*
 * Copyright 1997-2001 by George Greer (greerga@circlemud.org)		*
 ************************************************************************/
#pragma once

#include "structs.h"

extern void copy_object_strings(struct obj_data *to, struct obj_data *from);

extern void free_object_strings(struct obj_data *obj);

extern void free_object_strings_proto(struct obj_data *obj);

extern int copy_object(struct obj_data *to, struct obj_data *from);

extern int copy_object_preserve(struct obj_data *to, struct obj_data *from);

extern int save_objects(zone_rnum vznum);

extern obj_rnum insert_object(struct obj_data *obj, obj_vnum ovnum);

extern obj_rnum index_object(struct obj_data *obj, obj_vnum ovnum, obj_rnum ornum);

extern int update_objects(struct obj_data *refobj);

extern obj_rnum add_object(struct obj_data *, obj_vnum ovnum);

extern int delete_object(obj_rnum);
