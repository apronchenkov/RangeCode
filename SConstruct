env = Environment()
env['CC']='g++-4.6'
env['CXX']='g++-4.6'
env.Append(CCFLAGS=['-g', '-Wall', '-Wextra', '-pedantic', '-O3'])
env.Append(CXXFLAGS=['-g', '-std=c++0x', '-Wall', '-Wextra', '-pedantic', '-O3'])

env.Program(source=['ae.cpp'])
env.Program(source=['ad.cpp'])

env.Program(source=['qe.cpp'])
env.Program(source=['qd.cpp'])
