project('pfsshell', 'c')

add_global_arguments(['-O0', '-g', '-Wall', '-Wno-unused-value', '-Wno-format'], language : 'c')

cc = meson.get_compiler('c')
host_system = host_machine.system()

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_noinclude_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_noinclude_dep')

libapa_proj = subproject('apa')
libapa_dep = libapa_proj.get_variable('libapa_dep')

libpfs_proj = subproject('pfs')
libpfs_dep = libpfs_proj.get_variable('libpfs_dep')

libiomanX_proj = subproject('iomanX')
libiomanX_dep = libiomanX_proj.get_variable('libiomanX_dep')

libhdlfs_proj = subproject('hdlfs')
libhdlfs_dep = libhdlfs_proj.get_variable('libhdlfs_dep')

pfsshell_src = [
	'startup.c',
	'hl.c',
	'util.c',
	'shell.c'
]
pfsshell_cflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	pfsshell_cflags += ['-DNEED_GETLINE']
endif
pfsshell_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	pfsshell_ldflags += ['-static']
endif
pfsshell_dependson = [
	libfakeps2sdk_noinclude_dep,
	libapa_dep,
	libpfs_dep,
	libiomanX_dep,
	libhdlfs_dep,
]
pfsshell = executable('pfsshell', sources: pfsshell_src, c_args: pfsshell_cflags, link_args: pfsshell_ldflags, dependencies: pfsshell_dependson, install : true)
