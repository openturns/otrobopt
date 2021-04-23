"""
    otrobopt --- An OpenTURNS module
    ==================================

    Contents
    --------
      'otrobopt' is a module for OpenTURNS

"""

import sys
if sys.platform.startswith('win'):
    # this ensures OT dll is loaded
    import openturns

from .otrobopt import *

__version__ = '0.9'
