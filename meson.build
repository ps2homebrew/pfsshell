project('pfsshell', 'c', default_options: ['optimization=0', 'debug=true', 'warning_level=0'], version: 'v1.1.1')

add_global_arguments(['-Wno-unused-value', '-Wno-format', '-Wno-format-security', '-Wno-unused-function', '-Wno-unused-variable'], language : 'c')

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
	'src/startup.c',
	'src/hl.c',
	'src/host_adapter.c',
	'src/util.c',
	'src/shell.c',
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

if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	#Build libdokanfuse using CMake
	run_command('./dokany.sh')
	libfuse_include = include_directories(['external/dokany/dokan_fuse/include'])
	libfuse_dep = cc.find_library('libdokanfuse2.dll',
               dirs : [meson.current_source_dir() +'/external/dokany/dokan_fuse/build'])
else
	libfuse_include = ['']
	libfuse_dep = dependency('fuse')
endif

pfsfuse_src = [
	'src/iomanx_adapter.c',
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
pfsfuse = executable('pfsfuse', sources: pfsfuse_src, include_directories : libfuse_include, c_args: pfsfuse_cflags, link_args: pfsfuse_ldflags, dependencies: pfsfuse_dependson, install: true)
endif

if get_option('enable_pfs2tar') == true
pfs2tar_src = [
    'src/pfs2tar.c',
]
pfs2tar_cflags = []
if host_system == 'linux'
    pfs2tar_cflags += ['-pthread', '-D_FILE_OFFSET_BITS=64']
elif host_system == 'darwin'
    # macOS-specific adjustments
    # Threads are part of libc, no need for -pthread
    # No need for -D_FILE_OFFSET_BITS=64
endif

pfs2tar_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
    pfs2tar_ldflags += ['-static']
elif host_system == 'linux'
    pfs2tar_ldflags += ['-lpthread']
endif

pfs2tar_dependson = [
    libfakeps2sdk_noinclude_dep,
    libapa_dep,
    libpfs_dep,
    libiomanX_dep,
]

pfs2tar = executable(
    'pfs2tar',
    sources: pfs2tar_src,
    c_args: pfs2tar_cflags,
    link_args: pfs2tar_ldflags,
    dependencies: pfs2tar_dependson,
    install: true
)
endif

if get_option('enable_pfsd') == true
pfsd_src = [
	'src/pfsd.c',
	'src/pfslib_compat.c',
]
pfsd_cflags = []
if host_system == 'linux'
	pfsd_cflags += ['-pthread']
endif
pfsd_cflags += ['-D_FILE_OFFSET_BITS=64']
pfsd_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	pfsd_ldflags += ['-static']
endif
if host_system == 'linux'
	pfsd_ldflags += ['-lpthread']
endif
pfsd_dependson = [
	libfakeps2sdk_noinclude_dep,
	libapa_dep,
	libpfs_dep,
	libiomanX_dep,
]
pfsd = executable('pfsd', sources: pfsd_src, c_args: pfsd_cflags, link_args: pfsd_ldflags, dependencies: pfsd_dependson, install: true)
endif

if get_option('enable_ps2kinst') == true
ps2kinst_src = [
	'src/pfsd_client.c',
	'src/pfslib_compat.c',
	'src/ps2kinst.c',
]
ps2kinst_cflags = []
if host_system == 'linux'
	ps2kinst_cflags += ['-pthread']
endif
ps2kinst_cflags += ['-D_FILE_OFFSET_BITS=64']
ps2kinst_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
	ps2kinst_ldflags += ['-static']
endif
if host_system == 'linux'
	ps2kinst_ldflags += ['-lpthread']
endif
ps2kinst_dependson = [
	libfakeps2sdk_noinclude_dep,
	libapa_dep,
	libpfs_dep,
	libiomanX_dep,
]
ps2kinst = executable('ps2kinst', sources: ps2kinst_src, c_args: ps2kinst_cflags, link_args: ps2kinst_ldflags, dependencies: ps2kinst_dependson, install: true)
endif

if host_system == 'windows' and cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
archive = custom_target(
    'archive',
    output: meson.project_name() + '-' + meson.project_version() + '-win32.7z',
    input: [pfsshell, libps2hdd, pfs2tar, 'README.md', 'COPYING', 'COPYING.AFLv2'],
    build_by_default: true,
    command: [find_program('7z'), 'a', '@OUTPUT@', '@INPUT@']
)
endif
