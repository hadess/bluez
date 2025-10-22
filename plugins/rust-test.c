#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "src/plugin.h"

extern int rust_test_init(void);
extern void rust_test_exit(void);

static int __rust_test_init(void)
{
	return rust_test_init();
}

static void __rust_test_exit(void)
{
	return rust_test_exit();
}

BLUETOOTH_PLUGIN_DEFINE(rust_test, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
						__rust_test_init, __rust_test_exit)
