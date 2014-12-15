#if !defined (_DBG_H)
#  define _DBG_H

#if 1
#  define dbg_printf _dbg_printf
#else
#  define dbg_printf(...)
#endif

void _dbg_printf (const char *mask,...);

#endif /* _DBG_H defined? */
