#!/usr/bin/env python3
import argparse,json,zipfile,plistlib,shutil,time
from pathlib import Path
from common import ROOT,APP,ENGINE,BRIDGE,ART,state,digest_file,digest_stream,macho_identity

def package(baseline,bridge,out,art,identities):
 baseline,bridge,out,art=map(Path,(baseline,bridge,out,art))
 if out.resolve()in [baseline.resolve(),bridge.resolve(),art.resolve()]:raise ValueError('Output must not overwrite any input.')
 known={p['ipa_sha256']:p for p in identities['package_baselines']};key=digest_file(baseline)
 if key not in known:raise ValueError('Unrecognized unsigned baseline SHA-256. Use an archived, verified 0.2.18/0.2.19 baseline; do not bypass the identity check.')
 identity=known[key];native=macho_identity(bridge.read_bytes())
 if native['cpu']!=0x100000c or native['install_name']!='@rpath/DuoSceneBridge.dylib':raise ValueError('Expected an arm64 DuoSceneBridge dylib with its reviewed install name.')
 with zipfile.ZipFile(baseline)as z:
  if len(z.namelist())!=len(set(z.namelist())):raise ValueError('Duplicate archive paths.')
  if BRIDGE not in z.namelist():raise ValueError('Baseline is missing the prior bridge.')
  with z.open(ENGINE)as f:header=f.read(8192)
  if macho_identity(header)['uuid_hex']!=identity['engine_uuid_hex']:raise ValueError('Engine UUID mismatch.')
  with z.open(ENGINE)as f:
   if digest_stream(f)!=identity['engine_sha256']:raise ValueError('Engine bytes mismatch.')
  info=plistlib.loads(z.read(APP+'Info.plist'));configs=info.get('UIApplicationSceneManifest',{}).get('UISceneConfigurations',{})
  if not any(c.get('UISceneDelegateClassName')=='DuoSceneDelegate'for values in configs.values()for c in values):raise ValueError('Missing scene bridge configuration.')
 out.parent.mkdir(parents=True,exist_ok=True)
 with zipfile.ZipFile(baseline)as a,zipfile.ZipFile(out,'w',allowZip64=True)as b:
  for item in a.infolist():
   if item.filename in [BRIDGE,ART]:b.writestr(item,bridge.read_bytes()if item.filename==BRIDGE else art.read_bytes())
   else:
    with a.open(item)as src,b.open(item,'w')as dest:shutil.copyfileobj(src,dest,1024*1024)
  if ART not in a.namelist():
   item=zipfile.ZipInfo(ART,time.localtime()[:6]);item.create_system=3;item.external_attr=0o100644<<16;item.compress_type=zipfile.ZIP_DEFLATED;b.writestr(item,art.read_bytes())
 with zipfile.ZipFile(baseline)as a,zipfile.ZipFile(out)as b:
  changed=[n for n in a.namelist()if a.getinfo(n).CRC!=b.getinfo(n).CRC];added=[n for n in b.namelist()if n not in a.namelist()]
  if any(n not in [BRIDGE,ART]for n in changed)or any(n!=ART for n in added):raise AssertionError('Unexpected changed archive entries.')
  if b.testzip()is not None:raise AssertionError('ZIP integrity failed.')
 report={'source_sha256':key,'output_sha256':digest_file(out),'bridge_sha256':digest_file(bridge),'changed_entries':changed,'added_entries':added,'zip_integrity':True,'signing':'unsigned; re-sign app and nested bridge using the existing signing process','device_execution_verified':False}
 out.with_suffix(out.suffix+'.report.json').write_text(json.dumps(report,ensure_ascii=False,indent=2));return report

def main():
 p=argparse.ArgumentParser();p.add_argument('--baseline',type=Path,required=True);p.add_argument('--bridge',type=Path,required=True);p.add_argument('--out',type=Path,required=True);args=p.parse_args()
 print(json.dumps(package(args.baseline,args.bridge,args.out,ROOT/'assets/FakeLuStudio.jpg',state()),ensure_ascii=False,indent=2))
if __name__=='__main__':main()
