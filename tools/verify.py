#!/usr/bin/env python3
import re,json,hashlib,argparse
from pathlib import Path
from urllib.parse import unquote
from common import ROOT,state

def main():
 p=argparse.ArgumentParser();p.add_argument('--allow-source-changes',action='store_true');a=p.parse_args();errors=[]
 files=[f for f in ROOT.rglob('*')if f.is_file()and not any(x in ['.git','build','__pycache__']for x in f.relative_to(ROOT).parts)]
 for f in files:
  rel=f.relative_to(ROOT).as_posix()
  if f.suffix in ['.ipa','.dylib','.ips','.p12','.p8','.mobileprovision']or f.name in ['UnityFramework','global-metadata.dat']:errors.append('Unexpected private/binary input: '+rel)
  if f.suffix=='.json':
   try:json.loads(f.read_text())
   except Exception as e:errors.append(f'{rel}: {e}')
  if f.suffix=='.md':
   text=re.sub(r'```.*?```','',f.read_text(),flags=re.S)
   for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)',text):
    if re.match(r'\w+://',target)or target.startswith('#'):continue
    target=unquote(target.split('#')[0].split(' "')[0])
    if not (f.parent/target).exists():errors.append(f'Broken link in {rel}: {target}')
  if f.suffix in ['.md','.json','.m','.inc','.h']:
   text=f.read_text()
   if re.search(r'/Users/[A-Za-z0-9_.-]+/',text):errors.append('Personal absolute home path: '+rel)
 snap=json.loads((ROOT/'manifests/source-snapshot-0.2.19.json').read_text());mismatches=[]
 for rel,expected in snap.items():
  path=ROOT/rel
  if not path.exists()or hashlib.sha256(path.read_bytes()).hexdigest()!=expected:mismatches.append(rel)
 if mismatches and not a.allow_source_changes:errors.append('Source differs from archive snapshot: '+', '.join(mismatches))
 st=state()
 if st['preferences']['layouts']!='DuoUnifiedLayouts018':errors.append('Layout preference compatibility changed.')
 result={'files_checked':len(files),'source_snapshot_matches':not mismatches,'errors':errors,'device_execution_verified':False}
 print(json.dumps(result,ensure_ascii=False,indent=2))
 if errors:raise SystemExit(1)
if __name__=='__main__':main()
