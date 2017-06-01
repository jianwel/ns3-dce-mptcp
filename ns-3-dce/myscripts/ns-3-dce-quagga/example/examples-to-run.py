#! /usr/bin/env python
## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# A list of C++ examples to run in order to ensure that they remain
# buildable and runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run, do_valgrind_run).
#
# See test.py for more information.
cpp_examples = [
    ("dce-zebra-simple", "True", "True"),
    ("dce-quagga-ospfd-rocketfuel", "False", "False"),
    ("dce-quagga-bgpd-caida", "False", "False"),
    ("dce-quagga-bgpd", "True", "True"),
    ("dce-quagga-ospf6d --netStack=linux", "True", "True"),
    ("dce-quagga-ospfd", "True", "True"),
    ("dce-quagga-radvd", "True", "True"),
    ("dce-quagga-ripd", "True", "True"),
    ("dce-quagga-ripngd", "True", "True"),
]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
