project('iomanX', 'c')

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_dep')

libiomanX_inc = [
	include_directories('.'),
	include_directories('include'),
]

libiomanX_src = [
	'src/ioman_sbv.c',
	'src/iomanX.c',
]
libiomanX = static_library(
	'iomanX',
	sources: libiomanX_src,
	include_directories: libiomanX_inc,
	dependencies: libfakeps2sdk_dep
)

libiomanX_dep = declare_dependency(include_directories: libiomanX_inc, link_with: libiomanX)
