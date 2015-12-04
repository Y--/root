# -*- coding:utf-8 -*-
#-----------------------------------------------------------------------------
#  Copyright (c) 2015, ROOT Team.
#  Authors: Omar Zapata <Omar.Zapata@cern.ch> http://oproject.org
#           Danilo Piparo <Danilo.Piparo@cern.ch> CERN
#           Enric Tejedor enric.tejedor.saavedra@cern.ch> CERN
#  website: http://oproject.org/ROOT+Jupyter+Kernel (information only for ROOT kernel)
#  Distributed under the terms of the Modified LGPLv3 License.
#
#  The full license is in the file COPYING.rst, distributed with this software.
#-----------------------------------------------------------------------------
import sys
import os
from glob import glob
from tempfile import NamedTemporaryFile

from JupyROOT.kernel.handler import LoadHandlers

try:
    from JupyROOT.utils import invokeAclic
    from JupyROOT.utils import isPlatformApple
except ImportError:
    raise Exception("Error: JupyROOT not found")

import ROOT

import __builtin__

_ioHandler = None
_Executor  = None
_Declarer  = None

def GetIOHandler():
    global _ioHandler
    if not _ioHandler:
        LoadHandlers()
        from ROOT import JupyROOTExecutorHandler
        _ioHandler = JupyROOTExecutorHandler()
    return _ioHandler

def GetExecutor():
    global _Executor
    if not _Executor:
        from ROOT import JupyROOTExecutor
        _Executor = JupyROOTExecutor
    return _Executor

def GetDeclarer():
    global _Declarer
    if not _Declarer:
        from ROOT import JupyROOTDeclarer
        _Declarer = JupyROOTDeclarer
    return _Declarer


def ACLiC(code):
     tmpfile = NamedTemporaryFile(delete=False,suffix='.C',dir=os.getcwd())#will be removed when library is created
     tmpfile.write(code)
     tmpfilename = tmpfile.name
     tmpfile.close()
     status = 0
     if isPlatformApple():
         invokeAclic(tmpfilename)
     else:
         Executor = GetExecutor()
         status = Executor('.L %s+'%tmpfilename)
     return status

class MagicLoader(object):
    '''Class to load JupyROOT Magics'''
    def __init__(self,kernel):
         magics_path = os.path.dirname(__file__)+"/magics/*.py"
         for file in glob(magics_path):
              if file != magics_path.replace("*.py","__init__.py"):
                  module_path="JupyROOT.kernel.magics."+file.split("/")[-1].replace(".py","")
                  try:
                      module= __builtin__.__import__(module_path, globals(), locals(), ['register_magics'], -1)
                      module.register_magics(kernel)
                  except ImportError:
                      raise Exception("Error importing Magic: %s"%module_path)



