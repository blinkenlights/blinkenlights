
#ifndef __b_marshal_MARSHAL_H__
#define __b_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,INT (./bmarshal.list:3) */
extern void b_marshal_VOID__STRING_INT (GClosure     *closure,
                                        GValue       *return_value,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint,
                                        gpointer      marshal_data);

G_END_DECLS

#endif /* __b_marshal_MARSHAL_H__ */

