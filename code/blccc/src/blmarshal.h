
#ifndef __bl_marshal_MARSHAL_H__
#define __bl_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT (./blmarshal.list:1) */
#define bl_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:POINTER,INT (./blmarshal.list:2) */
extern void bl_marshal_VOID__POINTER_INT (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* VOID:POINTER (./blmarshal.list:3) */
#define bl_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:POINTER,BOOLEAN (./blmarshal.list:4) */
extern void bl_marshal_VOID__POINTER_BOOLEAN (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

/* VOID:UINT,POINTER (./blmarshal.list:5) */
#define bl_marshal_VOID__UINT_POINTER	g_cclosure_marshal_VOID__UINT_POINTER

/* VOID:VOID (./blmarshal.list:6) */
#define bl_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

G_END_DECLS

#endif /* __bl_marshal_MARSHAL_H__ */

