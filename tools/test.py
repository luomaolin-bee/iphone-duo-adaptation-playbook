#!/usr/bin/env python3
import argparse,platform,subprocess,sys,json
from common import ROOT

def run(command):
 r=subprocess.run([str(x)for x in command],cwd=ROOT,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 print(r.stdout,end='');r.check_returncode();return r.stdout.strip()
def main():
 p=argparse.ArgumentParser();p.add_argument('--portable-only',action='store_true');a=p.parse_args();out=ROOT/'build';out.mkdir(exist_ok=True);checks=[]
 checks.append(run([sys.executable,'-m','unittest','discover','-s','tests','-p','test_*.py','-v']))
 import shutil
 compiler=shutil.which('clang')or shutil.which('cc')
 if compiler:
  binary=out/'controller-select-test';run([compiler,'-std=c99','-Wall','-Wextra','-Werror','-I',ROOT/'src',ROOT/'tests/controller-select-test.c','-o',binary]);checks.append(run([binary]))
 else:checks.append('Controller Select policy C test skipped: no C compiler.')
 if not a.portable_only and platform.system()=='Darwin':
  native=[('model-test.m',['-fobjc-arc','-Wno-unused-function','-Wno-unused-parameter','-I',ROOT/'src','-framework','Foundation','-framework','CoreGraphics']),('feedback-test.c',['-I',ROOT/'src']),('metrics-test.c',[])]
  for source,flags in native:
   binary=out/Path(source).stem;run(['xcrun','clang','-Wall','-Wextra','-Werror',*flags,ROOT/'tests'/source,'-o',binary]);checks.append(run([binary]))
  if platform.machine()in ['arm64','aarch64']:
   for stem in ['tweak-test','menu-test']:
    obj=out/(stem+'.o');binary=out/stem
    run(['xcrun','clang','-arch','arm64','-x','assembler-with-cpp','-I',ROOT/'tests','-c',ROOT/'tests'/(stem+'.s'),'-o',obj])
    run(['xcrun','clang','-arch','arm64','-Wall','-Wextra','-Werror',ROOT/'tests'/(stem+'.c'),obj,'-o',binary]);checks.append(run([binary]))
  else:checks.append('ARM64 assembly execution skipped on this host architecture.')
 else:checks.append('Native macOS/iOS tests not executed in portable-only/non-Darwin mode.')
 report={'checks':checks,'device_execution_verified':False};(out/'test-report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2))
if __name__=='__main__':
 from pathlib import Path
 main()
