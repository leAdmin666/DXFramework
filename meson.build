project('DXEngine', 'c')

engine_src = ['src/DXEngine.c']

framework_subproject = subproject('DXFramework')
framework_subproject_dep = framework_subproject.get_variable('framework_subproject_dep')

executable('DXEngine', engine_src, dependencies: framework_subproject_dep)