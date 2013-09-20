env = Environment()
env['CC']='g++-4.6'
env['CXX']='g++-4.6'
#env['CC']='/opt/intel/composer_xe_2013_sp1.0.080/bin/intel64/icpc'
#env['CXX']='/opt/intel/composer_xe_2013_sp1.0.080/bin/intel64/icpc'
env.Append(CCFLAGS=['-g', '-Wall', '-Wextra', '-pedantic', '-O3'])
env.Append(CXXFLAGS=['-g', '-std=c++0x', '-Wall', '-Wextra', '-pedantic', '-O3'])
env.Append(LINKFLAGS=['-g'])

env.Program(source=['ae.cpp'])
env.Program(source=['ad.cpp'])

env.Program(source=['qe.cpp'], LIBS=['perfmon'])
env.Program(source=['qd.cpp'])
