project('pfs', 'c')

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_dep')

libiomanX_proj = subproject('iomanX')
libiomanX_dep = libiomanX_proj.get_variable('libiomanX_dep')

libpfs_inc = [
	include_directories('.'),
	include_directories('include'),
	include_directories('irx')
]
libpfs_src = [
	'_init.c',
	'irx/pfs.c',
	'irx/pfs_fio.c',
	'irx/pfs_fioctl.c',
	'src/bitmap.c',
	'src/block.c',
	'src/blockWrite.c',
	'src/cache.c',
	'src/dir.c',
	'src/inode.c',
	'src/journal.c',
	'src/misc.c',
	'src/super.c',
	'src/superWrite.c',
]
libpfs_deps = [
	libfakeps2sdk_dep,
	libiomanX_dep,
]
libpfs_c_args = [
	'-DPFS_POSIX_VER',
]
libpfs = static_library(
	'pfs',
	sources: libpfs_src,
	include_directories: libpfs_inc,
	dependencies: libpfs_deps,
	c_args: libpfs_c_args
)

libpfs_dep = declare_dependency(include_directories: libpfs_inc, link_with: libpfs)
