project('hdlfs', 'c')

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_dep')

libhdlfs_inc = [
	include_directories('.')
]
libhdlfs_src = [
	'_init.c',
	'main.c'
]
libhdlfs = static_library('hdlfs', sources: libhdlfs_src, include_directories: libhdlfs_inc, dependencies: libfakeps2sdk_dep)

libhdlfs_dep = declare_dependency(include_directories: libhdlfs_inc, link_with: libhdlfs)
