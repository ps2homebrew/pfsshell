project('fakeps2sdk', 'c')

libfakeps2sdk_inc = [
	include_directories('include')
]
libfakeps2sdk_src = [
	'atad.c',
	'thsemap.c',
]
libfakeps2sdk = static_library('fakeps2sdk', sources: libfakeps2sdk_src, include_directories: libfakeps2sdk_inc)

libfakeps2sdk_dep = declare_dependency(include_directories: libfakeps2sdk_inc, link_with: libfakeps2sdk)
libfakeps2sdk_noinclude_dep = declare_dependency(link_with: libfakeps2sdk)
