project('hdlfs', 'c')

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_dep')

libiomanX_proj = subproject('iomanX')
libiomanX_dep = libiomanX_proj.get_variable('libiomanX_dep')

libhdlfs_inc = [
	include_directories('.')
]
libhdlfs_src = [
	'_init.c',
	'main.c'
]
libapa_deps = [
	libfakeps2sdk_dep,
	libiomanX_dep,
]
libhdlfs = static_library(
	'hdlfs',
	sources: libhdlfs_src,
	include_directories: libhdlfs_inc,
	dependencies: libapa_deps
)

libhdlfs_dep = declare_dependency(include_directories: libhdlfs_inc, link_with: libhdlfs)
