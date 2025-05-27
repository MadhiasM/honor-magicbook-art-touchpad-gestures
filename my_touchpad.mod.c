#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb2768508, "__hid_register_driver" },
	{ 0x8fd70b55, "input_event" },
	{ 0x90b8f13a, "hid_unregister_driver" },
	{ 0xac19d52e, "input_unregister_device" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x1ff55b19, "input_allocate_device" },
	{ 0x47dcddf6, "input_register_device" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x8d5e53af, "module_layout" },
};

MODULE_INFO(depends, "hid");

MODULE_ALIAS("hid:b0018g*v000035CCp00000104");

MODULE_INFO(srcversion, "6DF2A824DA643BDE7CD396F");
