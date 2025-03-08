project('apa', 'c')

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_dep')

libiomanX_proj = subproject('iomanX')
libiomanX_dep = libiomanX_proj.get_variable('libiomanX_dep')

libapa_inc = [
	include_directories('.'),
	include_directories('include'),
	include_directories('irx'),
]
libapa_src = [
	'_init.c',
	'irx/hdd_fio.c',
	'irx/hdd_blkio.c',
	'irx/hdd.c',
	'src/apa.c',
	'src/cache.c',
	'src/free.c',
	'src/journal.c',
	'src/misc.c',
	'src/password.c',
]
libapa_deps = [
	libfakeps2sdk_dep,
	libiomanX_dep,
]
libapa_c_args = [
	'-DAPA_POSIX_VER',
	'-DAPA_USE_ATAD',
]
libapa = static_library(
	'apa',
	sources: libapa_src,
	include_directories: libapa_inc,
	dependencies: libapa_deps,
	c_args: libapa_c_args
)

libapa_dep = declare_dependency(include_directories: libapa_inc, link_with: libapa)
