#!/usr/bin/env python3
from pathlib import Path
import platform,subprocess,plistlib,json
from common import ROOT

def main():
 if platform.system()!='Darwin':raise SystemExit('Native bridge build requires macOS and Xcode.')
 out=ROOT/'build';out.mkdir(exist_ok=True)
 sdk=Path(subprocess.check_output(['xcrun','--sdk','iphoneos','--show-sdk-path'],text=True).strip())
 headers=sdk/'System/Library/Frameworks/UIKit.framework/Headers'
 for name in ['UIViewReservedRegion.h','UIHingeInteraction.h','UIGlassEffect.h']:
  if not (headers/name).is_file():raise SystemExit(f'Target SDK lacks {name}; use the compatible SDK described in docs/06-build-and-package.md.')
 version=subprocess.check_output(['xcrun','--sdk','iphoneos','--show-sdk-version'],text=True).strip()
 base=['xcrun','--sdk','iphoneos','clang','-arch','arm64','-miphoneos-version-min=15.0','-fobjc-arc','-Wall','-Wextra','-Werror','-Wno-unused-parameter']
 command=base+['-dynamiclib']
 for framework in ['UIKit','Foundation','CoreGraphics','QuartzCore','Metal','CoreImage','ImageIO','MetalPerformanceShaders','GameController']:command+=['-framework',framework]
 command+=['-install_name','@rpath/DuoSceneBridge.dylib','-o',str(out/'DuoSceneBridge.dylib'),str(ROOT/'src/DuoSceneBridge.m')]
 subprocess.run(command,check=True,cwd=ROOT)
 subprocess.run(base+['--analyze','-o',str(out/'analyzer.plist'),str(ROOT/'src/DuoSceneBridge.m')],check=True,cwd=ROOT)
 result=plistlib.loads((out/'analyzer.plist').read_bytes())
 if result.get('diagnostics'):raise SystemExit('Clang analyzer returned diagnostics.')
 report={'sdk_version':version,'strict_compile':True,'analyzer_diagnostics':0,'output':'build/DuoSceneBridge.dylib','device_execution_verified':False}
 (out/'build-report.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
if __name__=='__main__':main()
