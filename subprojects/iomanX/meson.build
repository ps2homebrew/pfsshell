project('iomanX', 'c')

libfakeps2sdk_proj = subproject('fakeps2sdk')
libfakeps2sdk_dep = libfakeps2sdk_proj.get_variable('libfakeps2sdk_dep')

libiomanX_src = [
	'iomanX.c'
]
libiomanX = static_library('iomanX', sources: libiomanX_src, dependencies: libfakeps2sdk_dep)

libiomanX_dep = declare_dependency(link_with: libiomanX)
