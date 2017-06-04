## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os
import Options
import os.path
import ns3waf
import sys
# local modules


def options(opt):
    opt.tool_options('compiler_cc') 
    ns3waf.options(opt)

def configure(conf):
    ns3waf.check_modules(conf, ['core', 'network', 'internet'], mandatory = True)
    ns3waf.check_modules(conf, ['point-to-point', 'tap-bridge', 'netanim'], mandatory = False)
    ns3waf.check_modules(conf, ['wifi', 'point-to-point', 'csma', 'mobility'], mandatory = False)
    ns3waf.check_modules(conf, ['point-to-point-layout'], mandatory = False)
    ns3waf.check_modules(conf, ['topology-read', 'internet-apps', 'applications', 'visualizer'], mandatory = False)
    conf.check_tool('compiler_cc')

    conf.env.append_value('CXXFLAGS', '-I/usr/include/python2.6')
    conf.env.append_value('LINKFLAGS', '-pthread')
    conf.check (lib='dl', mandatory = True)

    conf.env['ENABLE_PYTHON_BINDINGS'] = True
    conf.env['NS3_ENABLED_MODULES'] = []
    ns3waf.print_feature_summary(conf)



def build_dce_tests(module, bld):
    module.add_runner_test(needed=['core', 'dce-quagga', 'internet', 'csma'],
                           source=['test/dce-quagga-test.cc'])

def build_dce_examples(module):
    dce_examples = [
                   ]
    for name,lib in dce_examples:
        module.add_example(**dce_kw(target = 'bin/' + name, 
                                    source = ['example/' + name + '.cc'],
                                    lib = lib))

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point', 'point-to-point-layout'],
                       target='bin/dce-zebra-simple',
                       source=['example/dce-zebra-simple.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point', 'internet-apps,' 'applications', 'topology-read'],
                       target='bin/dce-quagga-ospfd-rocketfuel',
                       source=['example/dce-quagga-ospfd-rocketfuel.cc'])

def build_dce_kernel_examples(module):
    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point'],
                       target='bin/dce-quagga-radvd',
                       source=['example/dce-quagga-radvd.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point'],
                       target='bin/dce-quagga-ripd',
                       source=['example/dce-quagga-ripd.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point'],
                       target='bin/dce-quagga-ripngd',
                       source=['example/dce-quagga-ripngd.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point'],
                       target='bin/dce-quagga-ospfd',
                       source=['example/dce-quagga-ospfd.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point'],
                       target='bin/dce-quagga-ospf6d',
                       source=['example/dce-quagga-ospf6d.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point', 'topology-read'],
                       target='bin/dce-quagga-bgpd-caida',
                       source=['example/dce-quagga-bgpd-caida.cc'])

    module.add_example(needed = ['core', 'internet', 'dce-quagga', 'point-to-point'],
                       target='bin/dce-quagga-bgpd',
                       source=['example/dce-quagga-bgpd.cc'])

def build(bld):
    module_source = [
        'helper/quagga-helper.cc',
        ]
    module_headers = [
        'helper/quagga-helper.h',
        ]
    module_source = module_source
    module_headers = module_headers
    uselib = ns3waf.modules_uselib(bld, ['core', 'network', 'internet', 'netlink', 'dce'])
    module = ns3waf.create_module(bld, name='dce-quagga',
                                  source=module_source,
                                  headers=module_headers,
                                  use=uselib,
                                  lib=['dl'])
#                                  lib=['dl','efence'])

    build_dce_tests(module,bld)
    build_dce_examples(module)
    build_dce_kernel_examples(module)
