project('pfsshell', 'c', default_options: ['optimization=0', 'debug=true', 'warning_level=1'], version: 'v1.1.1')

add_global_arguments(['-Wno-unused-value', '-Wno-format', '-Wno-unused-function', '-Wno-unused-variable'], language : 'c')

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
if host_system == 'linux'
	pfsshell_cflags += ['-pthread']
endif
pfsshell_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	pfsshell_ldflags += ['-static']
endif
if host_system == 'linux'
	pfsshell_ldflags += ['-lpthread']
endif
pfsshell_dependson = [
	libfakeps2sdk_noinclude_dep,
	libapa_dep,
	libpfs_dep,
	libiomanX_dep,
	libhdlfs_dep,
]
pfsshell = executable('pfsshell', sources: pfsshell_src, c_args: pfsshell_cflags, link_args: pfsshell_ldflags, dependencies: pfsshell_dependson, install: true)
libps2hdd = shared_library('ps2hdd', sources: pfsshell_src, c_args: pfsshell_cflags, link_args: pfsshell_ldflags, dependencies: pfsshell_dependson, install: true)

if get_option('enable_pfsfuse') == true
libfuse_dep = dependency('fuse')
pfsfuse_src = [
	'iomanx_adapter.c',
]
pfsfuse_cflags = []
if host_system == 'linux'
	pfsfuse_cflags += ['-pthread']
endif
pfsfuse_cflags += ['-D_FILE_OFFSET_BITS=64']
pfsfuse_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	pfsfuse_ldflags += ['-static']
endif
if host_system == 'linux'
	pfsfuse_ldflags += ['-lpthread']
endif
pfsfuse_dependson = [
	libfakeps2sdk_noinclude_dep,
	libapa_dep,
	libpfs_dep,
	libiomanX_dep,
	libfuse_dep,
]
pfsfuse = executable('pfsfuse', sources: pfsfuse_src, c_args: pfsfuse_cflags, link_args: pfsfuse_ldflags, dependencies: pfsfuse_dependson, install: true)
endif

if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
archive = custom_target(
    'archive',
    output: meson.project_name() + '-' + meson.project_version() + '-win32.7z',
    input: [pfsshell, libps2hdd, 'README.md', 'COPYING', 'COPYING.AFLv2'],
    build_by_default: true,
    command: [find_program('7z'), 'a', '@OUTPUT@', '@INPUT@']
)
endif
